
# node-kytea

  node-kytea は Kytea を Node.js から利用するための C++ Addon です。

# What is KyTea?

See [KyTea](http://www.phontron.com/kytea/index-ja.html).

## Usage

```javascript
var KyTea = require('kytea').KyTea;
var path = '/path/to/model';
var kt = new KyTea(path, { tagmax: 3 }, function(err){
  if(err) throw err;
  kytea.getAllTags("...", function(err,obj){
    for(var i =0; i< obj.length;i++){
      var word = obj[i].surf;
      var pos = obj[i].tags[0];
      var pron = obj[i].tags[1];
    }
  });
});
```

## Methods
	
### new KyTea(modelPath, options, callback)

`modelPath`で指定されたモデルを読み込み、KyTeaのオブジェクトを作成します。

* `modelPath`: モデルのファイルパス
* `options`*(optional)*: 解析オプションです。現在、以下のオプションをサポートしています。
  * `notag`: n個目のタグを推定しない。
  * `nounk`: 未知語の読み推定を行わない。(Default: false)
  * `tagmax`: 1単語あたりのタグの最大数。(Default: 3) 
  * `deftag`: サブワード辞書に存在しない未知語など、タグを与えられない単語のためのタグ.(Default: 'UNK')
  * `unkbeam`: 未知語の読み推定で利用するビーム幅。(Default: 50)
  * `unktag`: 辞書に存在しない単語に付与されるタグ。(Default: '')
* `callback`: モデルの読み込みが完了した時点でこの関数が呼ばれます。

### getWS(text, callback)

単語分割を実行します。

* `text`: 単語分割対象の文字列
* `callback`: 単語分割処理が完了した時点でこの関数が呼ばれます。引数はエラーオブジェクトと、分割された単語が格納された配列です。

例：

```javascript
kytea.getWS('',function(err,words){
  ...
});
```

### getTags(text, callback)
### getAllTags(text, callback)

タグ推定(品詞、読み)を実行します。getTags()の場合はタグが複数ある場合は信頼度が最も高い1つのみを取得し、getAllTags()の場合は全てのタグを取得します。

* `text`: タグ推定対象の文字列
* `callback`: タグ推定処理が完了した時点でこの関数が呼ばれます。引数はエラーオブジェクトと、タグ推定結果が格納された配列です。配列の各要素の構造は以下のようになっています。

```javascript
[
  {
    surf: '私', // 単語表記
    tags:[
      [
    	  ['代名詞', 3.6951145482572487],//タグと信頼度
    	  ['名詞', 3.7467785662991857]
    	],
    	[ 
    	  ['わたし', 2.3796118434353652],
    	  ['わたくし', -0.2574841759018055]
    	]
   ]
 }
]
```

## Install

node-kytea は現在、KyTea 0.4.0 での動作を確認しています。
インストールの前に、[こちら](http://www.phontron.com/kytea/index-ja.html)から KyTea 0.4.0 をダウンロードして、インストールしてください。

KyTea のヘッダ及びライブラのパスがインクルードパス、ライブラリパスに含まれるようにしたうえで、以下のコマンドを実行してください。

```shell
git clone git://github.com/hideo55/node-kytea.git
cd node-kytea
node-waf configure build
```

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