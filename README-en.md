
# node-kytea

node-kytea is C++ Addon of Node.js for using KyTea.

## What is KyTea?

See [http://www.phontron.com/kytea/](http://www.phontron.com/kytea/).

## Usage

```javascript
var Kytea = require('kytea').Kytea;
var path = '/path/to/model';
var kt = new Kytea(path, { tagmax: 3 }, function(err){
  if(err) throw err;
  kt.getAllTags("これはテストです。", function(err,obj){
    for(var i =0; i< obj.length;i++){
      var word = obj[i].surf;
      var pos = obj[i].tags[0];
      var pron = obj[i].tags[1];
    }
  });
});
```

## Methods
	
### new Kytea(modelPath, options, callback)

Load model that specified by `modelPath`, and create KyTea object.

* `modelPath`: The path to model file.
* `options`*(optional)*: Now, the following options are supported. 
  * `notag`: Skips a particular tag.(Default: \[\], \[1\] will skip the first tag)
  * `nounk`: Don't estimate the pronunciation of unkown words.(Default: false)
  * `tagmax`:  The maximum number of tags to print for one word.(Default: 3, 0 implies no limit) 
  * `deftag`: A tag for words that cannot be given any tag.(for example, unknown words that contain a character not in the subword dictionary)(Default: 'UNK')
  * `unkbeam`: The width of the beam to use in beam search for unknown words.(Default: 50)
  * `unktag`: A tag to append to indicate words not in the dictionary.(Default: '')
* `callback`: The function called when reading of the model file is completed.

### getWS(text, callback)

This function execute word segmentation.

* `text`: The target text of word segmentation.
* `callback`: The function called when word segmentation is completed.

Example：

```javascript
kytea.getWS('これはテストです。',function(err,words){
  ...
});
```

### getTags(text, callback)
### getAllTags(text, callback)

This function execute particular tag. 
getTags() is get most confidenceial tags, and getAllTags() is get all tags.

* `text`: The target text of particular tag.
* `callback`: The function called when particular tag is completed.

Example:

```javascript
kytea.getAllTags("これはテストです。", function(err, res){
  if(err) throw err;
  res.forEach(function(elm){
    var surf = elm.surf;
    var tags = elm.tags;
    tags.forEach(function(tag){
    	var tag_name = tag[0];
    	var confidence = tag[1];
    	...
    });
  });
});
```

```javascript
[
  {
    surf: 'これ', // surface
    tags:[
      [
        ['代名詞', 3.6951145482572487],// tag and confidence
        ['名詞', 3.7467785662991857]
      ],
      [
        ['これ', 2.3796118434353652],
      ]
    ]
  }
]
```

## Requirement

* Node >= v0.6.13
* KyTea >= 0.4.2

node-kytea is verifying with KyTea 0.4.2. 
Please download and install KyTea before installation. 

## Install

Please set a path to directory of Kytea's header and library files, and execute following commands.

```shell
npm install kytea
```

or

```shell
git clone git://github.com/hideo55/node-kytea.git
npm install ./node-kytea
```

## License 

(The MIT License)

Copyright (c) 2012 Hideaki Ohno &lt;hide.o.j55{at}gmail.com&gt;

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
'Software'), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
