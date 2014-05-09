var Kytea = require('../index').Kytea;
var assert = require('assert');

var path = '' + __dirname + '/data/test.mod';

// Exception test
describe('exception test', function() {

  it('Failed to open', function(done) {
    var kytea = new Kytea();
    kytea.open('./path/notexists', function(err) {
      assert.ok(err.message.match(/^Could not open model file \.\/path\/notexists$/), "Model opne error");
      done();
    });
  });

  it('getWS() - model is not load', function(done) {
    var kytea = new Kytea();
    kytea.getWS('hoge', function(err) {
      assert.ok(err.message.match(/^Model is not loaded$/), "Model unloaded error in getWS()");
      done();
    });
  });

  it('getTags() - model is not load', function(done) {
    var kytea = new Kytea();
    kytea.getTags('hoge', function(err) {
      assert.ok(err.message.match(/^Model is not loaded$/), "Model unloaded error in getTags()");
      done();
    });
  });

  it('getAllTags() - model is not load', function(done) {
    var kytea = new Kytea();
    kytea.getAllTags('hoge', function(err) {
      assert.ok(err.message.match(/^Model is not loaded$/), "Model unloaded error in getAllTags()");
      done();
    });
  });

  it('Too many arguments', function(done) {
    assert.throws(function() {
      new Kytea(0, 1);
    }, /Too many arguments/);
    done();
  });

  it('Argument type error', function(done) {
    assert.throws(function() {
      new Kytea(0);
    }, /Argument 1 must be a Object/);
    done();
  });

  describe('Options', function() {
    it("'notag' value type invalid. Pattern - not Array", function(done) {
      assert.throws(function() {
        new Kytea({
          notag : 1
        });
      }, /Option "notag" must be a array of integer/);
      done();
    });

    it("'notag' value type invalid. Pattern - Array element is not Interger", function(done) {
      assert.throws(function() {
        new Kytea({
          notag : ['foo']
        });
      }, /Option "notag" must be a array of integer/);
      done();
    });

    it("'notag' value type invalid. Pattern- Array element less than 1", function(done) {
      assert.throws(function() {
        new Kytea({
          notag : [0]
        });
      }, /Illegal setting for "notag" \(must be 1 or greater\)/);
      done();
    });

    it("'tagmax' value type invalid. Pattern - not Integer", function(done) {
      assert.throws(function() {
        new Kytea({
          tagmax : "foo"
        });
      }, /Option "tagmax" must be a Integer/);
      done();
    });

    it("'unkbeam' value type invalid. Pattern - not Integer", function(done) {
      assert.throws(function() {
        new Kytea({
          unkbeam : "foo"
        });
      }, /Option "unkbeam" must be a Integer/);
      done();
    });

    it("'deftag' value type invalid. Pattern - not String", function(done) {
      assert.throws(function() {
        new Kytea({
          deftag : 1
        });
      }, /Option "deftag" must be a String/);
      done();
    });

    it("'unktag' value type invalid. Pattern - not Integer", function(done) {
      assert.throws(function() {
        new Kytea({
          unktag : 1
        });
      }, /Option "unktag" must be a String/);
      done();
    });

    it("'nounk' value type invalid. Pattern - not Boolean", function(done) {
      assert.throws(function() {
        new Kytea({
          nounk : 1
        });
      }, /Option "nounk" must be a Boolean/);
      done();
    });

  })
});

describe('normal test', function() {
  it('getWS()', function(done) {
    var kytea = new Kytea();
    kytea.open(path, function(err) {
      kytea.getWS("これはテストです。", function(err, res) {
        assert.deepEqual(res, ['これ', 'は', 'テスト', 'で', 'す', '。'], "getWS() test");
        done();
      });
    });
  });

  it('getTags()', function(done) {
    var kytea = new Kytea();
    kytea.open(path, function(err) {
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
        assert.deepEqual(res, expected, "getTags() test - normal");
        done();
      });
    });
  });

  it('getTags() - pronounce of number', function(done) {
    var kytea = new Kytea();
    kytea.open(path, function(err) {
      kytea.getTags("2012年", function(err, res) {
        var expected = [{
          surf : '2012',
          tags : [[['名詞', 100]], [['にせんじゅうに', 100]]]
        }, {
          surf : '年',
          tags : [[['接尾辞', 100]], [['ねん', 100]]]
        }];
        assert.deepEqual(res, expected, "getTags() test - number analyze");
        done();
      });
    });
  });

  it('getAllTags()', function(done) {
    var kytea = new Kytea();
    kytea.open(path, function(err) {
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
        assert.deepEqual(res, expected, "getAllTags() test");
        done();
      })
    });
  });
});

describe('notag test', function() {
  it('getTags() - 1', function(done) {
    var kytea = new Kytea({
      notag : [1]
    });
    kytea.open(path, function(err) {
      kytea.getTags("2012年", function(err, res) {
        var expected = [{
          surf : '2012',
          tags : [[], [['にせんじゅうに', 100]]]
        }, {
          surf : '年',
          tags : [[], [['ねん', 100]]]
        }];
        assert.deepEqual(res, expected, "notag test1");
        done();
      });
    });
  });

  it('getTags() - 2', function(done) {
    var kytea = new Kytea({
      notag : [2]
    });
    kytea.open(path, function(err) {
      kytea.getTags("2012年", function(err, res) {
        var expected = [{
          surf : '2012',
          tags : [[['名詞', 100]]]
        }, {
          surf : '年',
          tags : [[['接尾辞', 100]]]
        }];
        assert.deepEqual(res, expected, "notag test2");
        done();
      });
    });
  });

}); 