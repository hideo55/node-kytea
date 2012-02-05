
# node-kytea

  Node binding of KyTea.

# What is KyTea?

See [KyTea](http://www.phontron.com/kytea/index-ja.html)

## Usage

    var KyTea = require('kytea').KyTea;
    var path = '/path/to/model';
    var kt = new KyTea(path, { tagmax: 3 }, function(err){
      if(err) throw err;
      kytea.analyze("...", function(err,obj){
        for(var i =0; i< obj.length;i++){
          var word = obj[i].word;
          var tags = obj[i].tags;
        }
      });
    });

## Methods
	
### new KyTea(modelPath, options, callback)

returns new KyTea object and read model.

*`modelPath`: filepath of model

*`options`*(optional)*: analyze options 
  *`tagmax`: Maximum number of tag.(Integer)
  *`deftag`: Default tag string.(String)

*`callback`: This function will be called when model was opened.

### analyze(text, callback)

Analyzing string.

*`text`: analysis target

*`callback`: This function will be called when text analysis is completed.

## License 

(The MIT License)

Copyright (c) 2011 Hideaki Ohno &lt;hide.o.j55{at}gmail.com&gt;

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
