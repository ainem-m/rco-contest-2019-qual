# テストケースジェネレータ

## 複数テストケースをまとめて作る場合

#### `seeds.txt`の作成
[make_seeds.py](make_seeds.py)を使います。
14行目のSEEDの値を好きな値に変更し、
`python3 make_seeds.py <n> >seeds.txt`
n行のseed値が記された`seeds.txt`が生成されます。

#### 実行
[gen_from_seeds.py](gen_from_seeds.py)
13行目OUTPUT_DIRに出力先フォルダを設定し、
`python3 gen_from_seeds.py seeds.txt`

## 単一のテストケースを作る場合
[gen.py](gen.py)
#### ファイルに書き出す場合
`python3 gen.py <seed> >in.txt`
`in.txt`(ファイル名)は適宜変更してください。
<seed>は指定しないこともできます。
#### そのままプログラムに渡す場合
`python3 gen.py <seed> | python3 solve.py`
`gen.py`の出力結果をそのまま`solve.py`の標準入力に渡します。

