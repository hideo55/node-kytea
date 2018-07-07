
# node-kytea

node-kytea は単語分割、品詞推定、読み推定を行うテキスト解析器 Kytea を Node.js から利用するための C++ Addon です。

## What is KyTea?

See [http://www.phontron.com/kytea/index-ja.html](http://www.phontron.com/kytea/index-ja.html).

## Usage

```javascript
var Kytea = require('kytea').Kytea;
var path = '/path/to/model';
var kytea = new Kytea({ tagmax: 3 });
var kytea.open(path, function(err) {
  if(err) throw err;
  kytea.getAllTags("これはテストです。", function(err,obj){
    for(var i =0; i< obj.length;i++){
      var word = obj[i].surf;
      var pos = obj[i].tags[0];
      var pron = obj[i].tags[1];
    }
  });
});
```

## Methods
	
### new Kytea(options)

KyTeaのオブジェクトを作成します。

* `options`*(optional)*: 解析オプションです。現在、以下のオプションをサポートしています。
  * `notag`: n個目のタグを推定しない。(Default: \[\], \[1\]の場合は1番目のタグ推定がスキップされます)
  * `nounk`: 未知語の読み推定を行わない。(Default: false)
  * `tagmax`: 1単語あたりのタグの最大数。(Default: 3、0の場合は無制限) 
  * `deftag`: サブワード辞書に存在しない未知語など、タグを与えられない単語のためのタグ.(Default: 'UNK')
  * `unkbeam`: 未知語の読み推定で利用するビーム幅。(Default: 50)
  * `unktag`: 辞書に存在しない単語に付与されるタグ。(Default: '')

### open(modelPath, callback)

`modelPath`で指定されたモデルを読み込みます。

* `modelPath`: モデルのファイルパス
* `callback`: モデルの読み込みが完了した時点でこの関数が呼ばれます。

### getWS(text, callback)

単語分割を実行します。

* `text`: 単語分割対象の文字列
* `callback`: 単語分割処理が完了した時点でこの関数が呼ばれます。引数はエラーオブジェクトと、分割された単語が格納された配列です。

例：

```javascript
kytea.getWS('これはテストです。',function(err,words){
  ...
});
```

### getTags(text, callback)
### getAllTags(text, callback)

タグ推定(品詞、読み等)を実行します。getTags()の場合はタグが複数ある場合は信頼度が最も高い1つのみを取得し、getAllTags()の場合は全てのタグを取得します。

* `text`: タグ推定対象の文字列
* `callback`: タグ推定処理が完了した時点でこの関数が呼ばれます。引数はエラーオブジェクトと、タグ推定結果が格納された配列です。配列の各要素の構造は以下のようになっています。

例:

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

なお、コールバック関数の第２引数は以下のようなオブジェクトです。

```javascript
[
  {
    surf: 'これ', // 単語表記
    tags:[
      [// 1つ目のタグのリスト(この例では品詞)
        ['代名詞', 3.6951145482572487],//タグと信頼度
        ['名詞', 3.7467785662991857]
      ],
      [// 2つ目のタグのリスト(この例では読み) 
        ['これ', 2.3796118434353652],
      ]
    ]
  }
]
```

## Requirement

* 8.11 >= Node >= v0.8
* KyTea >= 0.4.2

最新の node-kytea は KyTea 0.4.7 での動作を確認しています。
インストールの前に、[こちら](http://www.phontron.com/kytea/index-ja.html)から KyTea をダウンロードして、インストールしてください。

## Install

KyTea のヘッダ及びライブラのパスを取得する際に`pkg-config`を使用するので、`kytea.pc`があるディレクトリパスを環境変数`PKG_CONFIG_PATH`に追加し、以下のコマンドを実行してください。

```shell
npm install kytea
```

または

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
