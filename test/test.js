var Kytea = require('../index').Kytea;

var path = '' + __dirname + '/data/test.mod';

exports.setUp = function(cb) {
  cb();
};

var testSeries = function(tests) {
  var makeCallback = function(index) {
    if (index < tests.length - 1) {
      return function() {
        tests[index].call(null, makeCallback(index + 1));
      };
    } else if ( index = tests.length - 1) {
      return function() {
        tests[index].call(null, function() {
        });
      };
    }
  };
  var fn = makeCallback(0);
  fn.call();
};

// Exception test
exports['exception_test'] = function(test) {
  var kytea = new Kytea('./path/notexists', function(err) {
    testSeries([
    function(next) {
      test.ok( err instanceof Error);
      test.ok(err.message.match(/^Could not open model file \.\/path\/notexists$/));
      next();
    },
    function(next) {
      kytea.getWS('hoge', function(err) {
        test.ok( err instanceof Error);
        test.ok(err.message.match(/^Model in not loaded$/));
        next();
      });
    },
    function(next) {
      kytea.getTags('hoge', function(err) {
        test.ok( err instanceof Error);
        test.ok(err.message.match(/^Model in not loaded$/));
        next();
      });
    },
    function(next) {
      kytea.getAllTags('hoge', function(err) {
        test.ok( err instanceof Error);
        test.ok(err.message.match(/^Model in not loaded$/));
        next();
      });
    },
    function(next) {
      test.throws(function() {
        new Kytea(path);
      }, /Invalid Argument/);
      next();
    },
    function(next) {
      test.throws(function() {
        new Kytea(path, {
          notag : 1
        }, function() {
        });
      }, /Option "notag" must be a array of integer/);
      next();
    },
    function(next) {
      test.throws(function() {
        new Kytea(path, {
          notag : ['foo']
        }, function() {
        });
      }, /Option "notag" must be a array of integer/);
      next();
    },
    function(next) {
      test.throws(function() {
        new Kytea(path, {
          notag : [0]
        }, function() {
        });
      }, /Illegal setting for "notag" \(must be 1 or greater\)/);
      next();
    },
    function(next) {
      test.throws(function() {
        new Kytea(path, {
          tagmax : "foo"
        }, function() {
        });
      }, /Option "tagmax" must be a Integer/);
      next();
    },
    function(next) {
      test.throws(function() {
        new Kytea(path, {
          unkbeam : "foo"
        }, function() {
        });
      }, /Option "unkbeam" must be a Integer/);
      next();
    },
    function(next) {
      test.throws(function() {
        new Kytea(path, {
          deftag : 1
        }, function() {
        });
      }, /Option "deftag" must be a String/);
      next();
    },
    function(next) {
      test.throws(function() {
        new Kytea(path, {
          unktag : 1
        }, function() {
        });
      }, /Option "unktag" must be a String/);
      next();
    },
    function(next) {
      test.throws(function() {
        new Kytea(path, {
          nounk : 1
        }, function() {
        });
      }, /Option "nounk" must be a Boolean/);
      next();
    },
    function(next) {
      test.throws(function() {
        new Kytea(path, {
          enable_h2f : 1
        }, function() {
        });
      }, /Option "enable_h2f" must be a boolean/);
      test.done();
    }]);
  });
};

// normal analyze test
exports['normal_test'] = function(test) {
  var kytea = new Kytea(path, function() {
    testSeries([
    function(next) {
      kytea.getWS("これはテストです。", function(err, res) {
        test.deepEqual(res, ['これ', 'は', 'テスト', 'で', 'す', '。']);
        next();
      });
    },
    function(next) {

      kytea.getTags("これはテストです。", function(err, res) {
        var expected = [{
          surf : 'これ',
          tags : [[['代名詞', 100]], [['これ', 100]]]
        }, {
          surf : 'は',
          tags : [[['助詞', 0.047801458212339325]], [['は', 100]]]
        }, {
          surf : 'テスト',
          tags : [[['名詞', 100]], [['てすと', 100]]]
        }, {
          surf : 'で',
          tags : [[['助動詞', 100]], [['で', 100]]]
        }, {
          surf : 'す',
          tags : [[['語尾', 100]], [['す', 100]]]
        }, {
          surf : '。',
          tags : [[['補助記号', 100]], [['。', 100]]]
        }];
        test.deepEqual(res, expected);
        next();
      });
    },
    function(next) {
      kytea.getTags("2012年", function(err, res) {
        var expected = [{
          surf : '2012',
          tags : [[['名詞', 100]], [['にせんじゅうに', 100]]]
        }, {
          surf : '年',
          tags : [[['接尾辞', 100]], [['ねん', 100]]]
        }];
        test.deepEqual(res, expected);
        next();
      });
    },
    function(next) {
      kytea.getAllTags("これはテスト", function(err, res) {
        var expected = [{
          surf : 'これ',
          tags : [[['代名詞', 100]], [['これ', 100]]]
        }, {
          surf : 'は',
          tags : [[['助詞', 0.047801458212339325], ['言いよどみ', 0]], [['は', 100]]]
        }, {
          surf : 'テスト',
          tags : [[['名詞', 100]], [['てすと', 100]]]
        }];
        test.deepEqual(res, expected);
        test.done();
      });
    }]);
  });
};
//enable_h2f => false
exports["enable_h2f_test"] = function(test) {
  var kytea = new Kytea(path, {
    enable_h2f : false
  }, function() {
    kytea.getTags("2012年", function(err, res) {
      var expected = [{
        surf : '2012',
        tags : [[['名詞', 100]], [['にせんじゅうに', 100]]]
      }, {
        surf : '年',
        tags : [[['接尾辞', 100]], [['ねん', 100]]]
      }];
      test.deepEqual(res, expected);
      test.done();
    });
  });
};

//notag test
exports['notag_test'] = function(test) {
  testSeries([
  function(next) {
    var kytea = new Kytea(path, {
      notag : [1]
    }, function() {
      kytea.getTags("2012年", function(err, res) {
        var expected = [{
          surf : '2012',
          tags : [[], [['にせんじゅうに', 100]]]
        }, {
          surf : '年',
          tags : [[], [['ねん', 100]]]
        }];
        test.deepEqual(res, expected);
        next();
      });
    });
  },
  function(next) {
    var kytea2 = new Kytea(path, {
      notag : [2]
    }, function(err) {
      test.equal(err, undefined);
      kytea2.getTags("2012年", function(err, res) {
        var expected = [{
          surf : '2012',
          tags : [[['名詞', 100]]]
        }, {
          surf : '年',
          tags : [[['接尾辞', 100]]]
        }];
        test.deepEqual(res, expected);
        test.done();
      });
    });
  }]);
};
