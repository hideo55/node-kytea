var assert = require('assert');
var Kytea = require('../index').Kytea;

tests(__dirname + '/data/test.mod');

function tests(path) {
  // Exception test
  (function() {
    var kytea = new Kytea('./path/notexists', function(err) {
      assert( err instanceof Error);
      assert(err.message.match(/^Could not open model file \.\/path\/notexists$/));
    });
    kytea.getWS('hoge', function(err) {
      assert( err instanceof Error);
      assert(err.message.match(/^Model in not loaded$/));
    });
    kytea.getTags('hoge', function(err) {
      assert( err instanceof Error);
      assert(err.message.match(/^Model in not loaded$/));
    });
    kytea.getAllTags('hoge', function(err) {
      assert( err instanceof Error);
      assert(err.message.match(/^Model in not loaded$/));
    });
    assert.throws(function() {
      new Kytea(path);
    }, /Invalid Argument/);
    assert.throws(function() {
      new Kytea(path, {
        notag : 1
      }, function() {
      });
    }, /Option "notag" must be a array of integer/);
    assert.throws(function() {
      new Kytea(path, {
        notag : ['foo']
      }, function() {
      });
    }, /Option "notag" must be a array of integer/);
    assert.throws(function() {
      new Kytea(path, {
        notag : [0]
      }, function() {
      });
    }, /Illegal setting for "notag" \(must be 1 or greater\)/);
    assert.throws(function() {
      new Kytea(path, {
        tagmax : "foo"
      }, function() {
      });
    }, /Option "tagmax" must be a Integer/);
    assert.throws(function() {
      new Kytea(path, {
        unkbeam : "foo"
      }, function() {
      });
    }, /Option "unkbeam" must be a Integer/);
    assert.throws(function() {
      new Kytea(path, {
        deftag : 1
      }, function() {
      });
    }, /Option "deftag" must be a String/);
    assert.throws(function() {
      new Kytea(path, {
        unktag : 1
      }, function() {
      });
    }, /Option "unktag" must be a String/);
    assert.throws(function() {
      new Kytea(path, {
        nounk : 1
      }, function() {
      });
    }, /Option "nounk" must be a Boolean/);
    assert.throws(function() {
      new Kytea(path, {
        enable_h2f : 1
      }, function() {
      });
    }, /Option "enable_h2f" must be a boolean/);
  })();
  // normal analyze test
  (function() {
    var kytea = new Kytea(path, function() {
      kytea.getWS("これはテストです。", function(err, res) {
        assert.deepEqual(res, ['これ', 'は', 'テスト', 'で', 'す', '。']);
      });
      kytea.getTags("これはテストです。", function(err, res) {
        var expected = [{
          surf : 'これ',
          tags : [[['代名詞', 100]], [['これ', 100]]]
        }, {
          surf : 'は',
          tags : [[['助詞', 100]], [['は', 100]]]
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
        assert.deepEqual(res, expected);
      });
      kytea.getTags("2012年", function(err, res) {
        var expected = [{
          surf : '2012',
          tags : [[['名詞', 100]], [['にせんじゅうに', 100]]]
        }, {
          surf : '年',
          tags : [[['接尾辞', 100]], [['ねん', 100]]]
        }];
        assert.deepEqual(res, expected);
      });
    });
  })();
  //enable_h2f => false
  (function() {
    var kytea = new Kytea(path, {
      enable_h2f : false
    }, function() {
      kytea.getTags("2012年", function(err, res) {
        var expected = [{
          surf : '2012',
          tags : [[['UNK', 0]], [['UNK', 0]]]
        }, {
          surf : '年',
          tags : [[['接尾辞', 100]], [['ねん', 100]]]
        }];
        assert.deepEqual(res, expected);
      });
    });
  })();

  //notag test
  (function() {
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
        assert.deepEqual(res, expected);
      });
    });
    var kytea2 = new Kytea(path, {
      notag : [2]
    }, function(err) {
      assert.equal(err, undefined);
      kytea2.getTags("2012年", function(err, res) {
        var expected = [{
          surf : '2012',
          tags : [[['名詞', 100]]]
        }, {
          surf : '年',
          tags : [[['接尾辞', 100]]]
        }];
        assert.deepEqual(res, expected);
      });
    });
  })();
}