# myproxy 
myproxy は C言語で記述されたプロキシソフトウェアです。
実装方法の異なるtype Bとtype Cがあります。

# 動作環境
macOS

# 利用方法
* サーバ側でproxy_X.c (Xはtype)　を実行する。
* クライアント側でブラウザにプロキシを設定する。

# 実装の違い
## type B
fork()で多重アクセスに対応。

## type C
selectシステムコールで多重アクセスに対応。
