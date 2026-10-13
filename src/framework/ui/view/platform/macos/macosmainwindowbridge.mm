/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "macosmainwindowbridge.h"

#include <Cocoa/Cocoa.h>
#include <QGuiApplication>
#include <QQmlProperty>
#include <QWindow>
#include <QtMath>

using namespace muse::ui;

struct MacOSMainWindowBridge::TitleBarObserver {
    CFRunLoopObserverRef runLoopObserver = nullptr;
    id pressMonitor = nil;
    NSEvent* lastPress = nil;
};

//! NOTE Core Animation commits what has changed (shows it) in an observer of the run loop with this order
static const CFIndex CORE_ANIMATION_COMMIT_ORDER = 2000000;

//! NOTE AppKit keeps the region by which a window is moved, and it is not told when a view becomes opaque
//! (or stops being so). It makes the region again when this property changes
static void resetWindowDragRegion(NSWindow* nsWindow)
{
    const BOOL isMovable = nsWindow.movableByWindowBackground;
    [nsWindow setMovableByWindowBackground:!isMovable];
    [nsWindow setMovableByWindowBackground:isMovable];
}

static NSWindow* nsWindowForQWindow(QWindow* qWindow)
{
    if (!qWindow) {
        return nullptr;
    }

    // winId() is an NSView only on the cocoa platform (not e.g. offscreen)
    if (QGuiApplication::platformName() != "cocoa") {
        return nullptr;
    }

    NSView* nsView = (__bridge NSView*)reinterpret_cast<void*>(qWindow->winId());
    NSWindow* nsWindow = [nsView window];
    return nsWindow;
}

MacOSMainWindowBridge::MacOSMainWindowBridge(QObject* parent)
    : MainWindowBridge(parent)
{
}

MacOSMainWindowBridge::~MacOSMainWindowBridge()
{
    setTitleBarObserved(false);
}

void MacOSMainWindowBridge::init()
{
    MainWindowBridge::init();

    uiConfiguration()->applyPlatformStyle(m_window);

    uiConfiguration()->currentThemeChanged().onNotify(this, [this]() {
        uiConfiguration()->applyPlatformStyle(m_window);
    });

    //! NOTE A window which is not fully opaque has a view which is not opaque (see updateTitleBarMerged)
    connect(m_window, &QWindow::opacityChanged, this, [this]() {
        NSWindow* nsWindow = nsWindowForQWindow(m_window);
        if (titleBarMerged() && nsWindow) {
            resetWindowDragRegion(nsWindow);
        }
    });

    updateTitleBarMerged();
}

bool MacOSMainWindowBridge::fileModified() const
{
    //! NOTE QWindow misses an API for this, so we'll do it ourselves.
    NSWindow* nsWindow = nsWindowForQWindow(m_window);
    if (!nsWindow) {
        return false;
    }

    return [nsWindow isDocumentEdited];
}

void MacOSMainWindowBridge::setFileModified(bool modified)
{
    NSWindow* nsWindow = nsWindowForQWindow(m_window);
    if (!nsWindow) {
        return;
    }

    if ([nsWindow isDocumentEdited] == modified) {
        return;
    }

    [nsWindow setDocumentEdited:modified];
    emit fileModifiedChanged();
}

void MacOSMainWindowBridge::setTitleBarMerged(bool merged)
{
    if (m_titleBarMergeRequested == merged) {
        return;
    }

    m_titleBarMergeRequested = merged;
    updateTitleBarMerged();
}

void MacOSMainWindowBridge::setTitleBarHeight(int height)
{
    if (titleBarHeight() == height) {
        return;
    }

    MainWindowBridge::setTitleBarHeight(height);

    layoutWindowButtons();
    updateTitleBarInsets();
}

bool MacOSMainWindowBridge::canMergeTitleBar() const
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    return nsWindowForQWindow(m_window) != nullptr;
#else
    return false;
#endif
}

void MacOSMainWindowBridge::updateTitleBarMerged()
{
    const bool merge = m_titleBarMergeRequested && canMergeTitleBar();
    if (merge == titleBarMerged()) {
        return;
    }

    NSWindow* nsWindow = nsWindowForQWindow(m_window);
    if (!nsWindow) {
        return;
    }

    if (!merge) {
        setTitleBarObserved(false);
    }

    //! NOTE Under the title bar, AppKit leaves the presses of the mouse to a view only if that view is opaque:
    //! through any other view it is the title bar which is pressed, and which moves the window.
    //! Qt takes the opacity of its view from the format of the window, and the app asks its windows
    //! with an alpha channel, for which this window has no use
    QSurfaceFormat format = m_window->requestedFormat();
    if (merge) {
        m_alphaBufferSizeBeforeMerge = format.alphaBufferSize();
        format.setAlphaBufferSize(0);
    } else {
        format.setAlphaBufferSize(m_alphaBufferSizeBeforeMerge);
    }
    m_window->setFormat(format);

#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    //! NOTE Qt sets the style of the window again from its flags (e.g. when the full screen mode is left),
    //! so it is by the flags that the content is extended under the title bar. The rest is not known to Qt
    m_window->setFlag(Qt::ExpandedClientAreaHint, merge);
    m_window->setFlag(Qt::NoTitleBarBackgroundHint, merge);
#endif

    //! NOTE ApplicationWindow keeps its content out of the title bar (Qt 6.9+): the padding of its content is bound
    //! to the safe area of the window. Written as a QML property, the padding takes the place of that binding
    QQmlProperty topPadding(m_window, "topPadding");
    if (topPadding.isValid()) {
        if (merge) {
            topPadding.write(0.0);
        } else {
            topPadding.reset();
        }
    }

    [nsWindow setTitleVisibility:merge ? NSWindowTitleHidden : NSWindowTitleVisible];

    setIsTitleBarMerged(merge);

    if (merge) {
        setTitleBarObserved(true);
    }

    layoutWindowButtons();
    updateTitleBarInsets();

    resetWindowDragRegion(nsWindow);

    if (!merge) {
        //! NOTE With its flags, Qt has taken the transparency of the title bar, which the style of the app sets
        uiConfiguration()->applyPlatformStyle(m_window);
    }
}

void MacOSMainWindowBridge::setTitleBarObserved(bool observed)
{
    if (m_titleBarObserver) {
        CFRunLoopObserverInvalidate(m_titleBarObserver->runLoopObserver);
        CFRelease(m_titleBarObserver->runLoopObserver);

        [NSEvent removeMonitor:m_titleBarObserver->pressMonitor];
        [m_titleBarObserver->pressMonitor release];
        [m_titleBarObserver->lastPress release];

        delete m_titleBarObserver;
        m_titleBarObserver = nullptr;
    }

    if (!observed) {
        return;
    }

    //! NOTE The system puts the buttons of the window back to their places each time it lays the title bar out
    //! (a new title, another state of the document, a resize, the full screen mode, ...). So they are put in the middle
    //! of the bar again at the end of each turn of the run loop: after the system has done its part,
    //! and before the result is shown
    m_titleBarObserver = new TitleBarObserver();
    m_titleBarObserver->runLoopObserver = CFRunLoopObserverCreateWithHandler(
        kCFAllocatorDefault, kCFRunLoopBeforeWaiting | kCFRunLoopExit, true, CORE_ANIMATION_COMMIT_ORDER - 1000,
        ^(CFRunLoopObserverRef, CFRunLoopActivity) {
            layoutWindowButtons();
            updateTitleBarInsets();
        });
    CFRunLoopAddObserver(CFRunLoopGetMain(), m_titleBarObserver->runLoopObserver, kCFRunLoopCommonModes);

    //! NOTE The last press of the mouse is kept for handleTitleBarPress
    TitleBarObserver* observer = m_titleBarObserver;
    id monitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskLeftMouseDown
                  handler:^NSEvent*(NSEvent* event) {
                      [observer->lastPress release];
                      observer->lastPress = [event retain];
                      return event;
                  }];
    m_titleBarObserver->pressMonitor = [monitor retain];
}

void MacOSMainWindowBridge::layoutWindowButtons()
{
    if (m_isLayingOutWindowButtons || !titleBarMerged() || titleBarHeight() <= 0) {
        return;
    }

    NSWindow* nsWindow = nsWindowForQWindow(m_window);
    if (!nsWindow) {
        return;
    }

    NSButton* closeButton = [nsWindow standardWindowButton:NSWindowCloseButton];
    NSButton* miniaturizeButton = [nsWindow standardWindowButton:NSWindowMiniaturizeButton];
    NSButton* zoomButton = [nsWindow standardWindowButton:NSWindowZoomButton];

    //! NOTE The buttons are in the view of the title bar, which is in a view of the height of the title bar,
    //! which is in the frame of the window, beside the content. (In the full screen mode they are in another window:
    //! the title bar which comes down with the menu bar.) Where it is not so, the buttons stay where the system puts them
    static const Class CONTAINER_CLASS = NSClassFromString(@"NSTitlebarContainerView");

    NSView* titleBarView = closeButton.superview;
    NSView* containerView = titleBarView.superview;
    NSView* frameView = containerView.superview;
    if (!frameView || frameView != nsWindow.contentView.superview
        || !CONTAINER_CLASS || ![containerView isKindOfClass:CONTAINER_CLASS]
        || miniaturizeButton.superview != titleBarView || zoomButton.superview != titleBarView) {
        return;
    }

    m_isLayingOutWindowButtons = true;

    //! NOTE What the system still has to lay out, it lays out now: not after the buttons have been placed
    [nsWindow layoutIfNeeded];

    //! NOTE That view gets the height of the bar, so that the buttons can be in the middle of the bar and be pressed there
    const CGFloat height = titleBarHeight();
    NSRect containerFrame = containerView.frame;
    containerFrame.origin.y = frameView.isFlipped ? 0. : NSHeight(frameView.bounds) - height;
    containerFrame.size.height = height;
    if (!NSEqualRects(containerView.frame, containerFrame)) {
        [containerView setFrame:containerFrame];
        [containerView layoutSubtreeIfNeeded];
    }

    //! NOTE As in a window with a toolbar: the buttons are (almost) as far from the side as from the top.
    //! They keep the order and the distance which the system gave them
    const CGFloat step = NSMinX(miniaturizeButton.frame) - NSMinX(closeButton.frame);
    if (qAbs(step) >= NSWidth(closeButton.frame)) {
        const CGFloat y = qRound((NSHeight(titleBarView.bounds) - NSHeight(closeButton.frame)) / 2);
        const CGFloat margin = y + 1;
        CGFloat x = step < 0 ? NSWidth(titleBarView.bounds) - margin - NSWidth(closeButton.frame) : margin;
        for (NSButton* button : { closeButton, miniaturizeButton, zoomButton }) {
            const NSPoint origin = NSMakePoint(x, y);
            if (!NSEqualPoints(button.frame.origin, origin)) {
                [button setFrameOrigin:origin];
            }

            x += step;
        }
    }

    m_isLayingOutWindowButtons = false;
}

void MacOSMainWindowBridge::updateTitleBarInsets()
{
    NSWindow* nsWindow = nsWindowForQWindow(m_window);

    //! NOTE In the full screen mode there is no title bar (the one which comes down with the menu bar is another window)
    if (!titleBarMerged() || !nsWindow || (nsWindow.styleMask & NSWindowStyleMaskFullScreen) != 0) {
        setTitleBarInsets(0, 0);
        return;
    }

    NSRect buttonsRect = NSZeroRect;
    for (NSWindowButton buttonType : { NSWindowCloseButton, NSWindowMiniaturizeButton, NSWindowZoomButton }) {
        NSButton* button = [nsWindow standardWindowButton:buttonType];
        if (button && button.window == nsWindow && !button.hiddenOrHasHiddenAncestor) {
            buttonsRect = NSUnionRect(buttonsRect, [button convertRect:button.bounds toView:nil]);
        }
    }

    //! NOTE The buttons are on the left, or on the right where the system is written from right to left
    const CGFloat windowWidth = NSWidth(nsWindow.frame);
    int leftInset = 0;
    int rightInset = 0;
    if (!NSIsEmptyRect(buttonsRect)) {
        if (NSMidX(buttonsRect) < windowWidth / 2) {
            leftInset = qCeil(NSMaxX(buttonsRect));
        } else {
            rightInset = qCeil(windowWidth - NSMinX(buttonsRect));
        }
    }

    setTitleBarInsets(leftInset, rightInset);
}

void MacOSMainWindowBridge::handleTitleBarPress()
{
    NSWindow* nsWindow = nsWindowForQWindow(m_window);
    if (!titleBarMerged() || !nsWindow || !m_titleBarObserver) {
        return;
    }

    //! NOTE The press which Qt is handling now. Qt gets it a bit later than AppKit, so it is not always the current event
    //! of the app any more: other events follow it at once (e.g. the ones of the pressure on a trackpad)
    static const NSTimeInterval PRESS_HANDLING_TIMEOUT = 1.;

    NSEvent* press = [m_titleBarObserver->lastPress autorelease];
    m_titleBarObserver->lastPress = nil;
    if (!press || press.window != nsWindow
        || [NSProcessInfo processInfo].systemUptime - press.timestamp > PRESS_HANDLING_TIMEOUT) {
        return;
    }

    if (press.clickCount == 2) {
        //! NOTE As a double click on a title bar: what is chosen in the settings of the system
        NSString* action = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleActionOnDoubleClick"];
        if ([action isEqualToString:@"Minimize"]) {
            [nsWindow performMiniaturize:nil];
        } else if (![action isEqualToString:@"None"]) {
            [nsWindow performZoom:nil];
        }
        return;
    }

    //! NOTE After a click there is nothing to move: the button has already been released
    const bool isButtonPressed = ([NSEvent pressedMouseButtons] & 1) != 0;
    if (press.clickCount != 1 || !isButtonPressed) {
        return;
    }

    [nsWindow performWindowDragWithEvent:press];

    //! NOTE From here the system moves the window, and the release of the button does not come to Qt,
    //! which would keep the button as pressed until the next click. So Qt gets a release when the move is over
    //! (the block waits for the default mode of the run loop, which is not the one of the move)
    NSView* nsView = (__bridge NSView*)reinterpret_cast<void*>(m_window->winId());
    CFRunLoopPerformBlock(CFRunLoopGetMain(), kCFRunLoopDefaultMode, ^{
        NSEvent* release = [NSEvent mouseEventWithType:NSEventTypeLeftMouseUp
                            location:nsWindow.mouseLocationOutsideOfEventStream
                            modifierFlags:0
                            timestamp:[NSProcessInfo processInfo].systemUptime
                            windowNumber:nsWindow.windowNumber
                            context:nil
                            eventNumber:0
                            clickCount:1
                            pressure:0];
        [nsView mouseUp:release];
    });
    CFRunLoopWakeUp(CFRunLoopGetMain());
}
