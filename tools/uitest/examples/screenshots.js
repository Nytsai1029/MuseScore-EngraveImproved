// An example of a script for tools/uitest/run.sh: saves pictures of the score page and of the style dialog.
//
//   tools/uitest/run.sh tools/uitest/examples/screenshots.js path/to/score.msdz
//
// The steps run one after another; a step which returns {code: N} with N other than 0 stops the script,
// and the app exits with that code.

const OUT_DIR = api.context.globalVal("out_dir")

var testCase = {
    name: "Screenshots of the score page and of the style dialog",
    steps: [
        {name: "Wait for the score to open", func: function() {
            api.autobot.sleep(4000)
        }},
        {name: "Score page", func: function() {
            api.autobot.saveScreenshots(OUT_DIR + "/1-score")
        }},
        {name: "Style dialog", func: function() {
            // Any action of the app can be dispatched; controls are reached with api.navigation and api.keyboard
            api.dispatcher.dispatch("edit-style")
            api.autobot.sleep(2500)
            api.autobot.saveScreenshots(OUT_DIR + "/2-style")
        }},
        {name: "Close the dialog", func: function() {
            api.navigation.escape()
            api.autobot.sleep(500)
        }}
    ]
}

function main()
{
    api.autobot.setInterval(200)
    api.autobot.runTestCase(testCase)
}
