# myproxy 
C言語で記述されたプロキシソフトウェア．
type Bとtype Cは実装方法が異なる．

# 動作環境
macOS

# 利用方法
* サーバ側でproxy_X.c (Xはtype)　を実行
* クライアント側でブラウザにプロキシを設定

# 実装の違い
## type B
fork()で多重アクセスに対応．

## type C
selectシステムコールで多重アクセスに対応．
