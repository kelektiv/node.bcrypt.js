var suites = {
  "core"   : {
    cases: require("./core").tests
    /*  Other options
    setUp    : function() { }, // called before every test
    tearDown : function() { }, // called after every test
    */
  }
};

require("mjsunit.runner/lib/runner").run(suites);