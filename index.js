var kytea = require('./build/Release/kytea').NodeKyTea;

var pattern = new RegExp('[A-Za-z0-9 ]', 'g');

var processWords = function(str, res) {
  var i = 0;
  res.forEach(function(v, index, arr) {
    var len = v.length;
    arr[index] = str.slice(i, i + len);
    i += len;
  });
};

var processTags = function(str, res) {
  var i = 0;
  res.forEach(function(v) {
    var len = v.surf.length;
    v.surf = str.slice(i, i + len);
    i += len;
  });
};

var createFilteredFunction = function(func, after) {
  return function(str, cb) {
    if(this.isEnableHalf2Full()) {
      var strFull = str.replace(pattern, function(s) {
        return String.fromCharCode(s.charCodeAt(0) + 0xFEE0);
      });
      func.call(this, strFull, function(err, res) {
        if(!err) {
          after(str, res);
        }
        cb(err, res);
      });
    } else {
      func.call(this, str, cb);
    }
  };
};

kytea.prototype.getWS = createFilteredFunction(kytea.prototype.getWS, processWords);
kytea.prototype.getTags = createFilteredFunction(kytea.prototype.getTags, processTags);
kytea.prototype.getAllTags = createFilteredFunction(kytea.prototype.getAllTags, processTags);

exports.KyTea = kytea;
