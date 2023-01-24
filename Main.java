import java.io.*;
import java.util.*;
import java.util.concurrent.ThreadLocalRandom;
import java.util.function.*;
import java.util.stream.IntStream;

public class Main{
  /* 定数 */
  static int infI = (int) 1e9;
  static long infL = (long) 1e18;
  long LMT = 1850;

  /* 入出力とか */
  long st = System.currentTimeMillis();
  MyReader in;
  MyWriter out;
  MyLogger log = new MyLogger();

  boolean local(){
    out.out = new ByteArrayOutputStream();
    return true;
  }

  /* 入力値 */
  int N;
  int M;
  static int[] A;

  /* 乱数・パラメータ */
  //  static Random rd = new Random(0);
  static Random rd = ThreadLocalRandom.current();
  //遷移で破壊するクラスタ数
  int seniN = 1;
  //遷移を何回試せたか
  int loop = 0;

  public Main(){ this(System.in,System.out); }

  public Main(InputStream ins,OutputStream outs){
    in = new MyReader(ins);
    out = new MyWriter(outs);
    N = in.it();
    M = in.it();
    A = in.it(N *N);
  }

  int[][] sur;
  int[] pos;

  private void preCalc(){
    setupSur();
    baseTeire = new int[N *N];
    pos = IntStream.range(0,N *N).filter(p -> 3 <= A[p]).toArray();
    //    calcBaseT();
    //    setupPos(9);
  }

  int[][] D = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};

  void setupSur(){
    sur = new int[N *N][];
    for (int i = 0;i < N;i++)
      for (int j = 0;j < N;j++) {
        List<Integer> list = new ArrayList<>();
        for (var d:D) {
          int ni = i +d[0];
          int nj = j +d[1];
          if (0 <= ni && 0 <= nj && ni < N && nj < N)
            list.add(top(ni,nj));
        }
        sur[top(i,j)] = list.stream().mapToInt(p -> p).toArray();
      }

  }

  int[] baseTeire;

  void calcBaseT(){
    for (int p = 0;p < N *N;p++) {
      int[] cnt = new int[11];
      for (var s:sur[p])
        cnt[A[s]]++;
      for (int shitu = Math.max(A[p],7);shitu < 10;shitu++) {
        int cost = shitu -A[p];
        if (10 -shitu < cnt[shitu])
          baseTeire[p] = cost;
      }
    }
    for (int p = 0;p < N *N;p++) {
      A[p] += baseTeire[p];
      M -= baseTeire[p];
    }
  }

  State solve(){
    preCalc();

    //初期解をいくつか作ってそれぞれやきなましてく
    Queue<Anneal> que = new ArrayDeque<>();
    for (int i = 0;i < 10;i++)
      que.add(new Anneal(init()));

    long elapsed;
    while ((elapsed = elapsed()) < LMT) {
      var ann = que.poll();
      for (int i = 0;i < 500;i++) {
        State nxt = nxt(ann.cur);
        //スコアが上がってるか確率で遷移する
        if (ann.cur.score < nxt.score || mutation(elapsed,ann.cur.score,nxt.score)) {
          ann.cur = nxt;
          if (ann.best.score < nxt.score) {
            ann.cnt = 0;
            ann.best = nxt;
          }
          ann.cnt++;
        }
        loop++;
        if (ann.cnt == 100)
          ann.cur = ann.best;
      }
      que.add(ann);
    }

    State ans = que.poll().best;
    while (!que.isEmpty()) {
      var tmp = que.poll();
      if (ans.score < tmp.best.score)
        ans = tmp.best;
    }

    return ans;
  }

  boolean mutation(long elapsed,double cur,double nxt){ return rd.nextDouble() *elapsed *nxt < LMT *cur; }

  State init(){
    State state = new State(0,new int[N *N],new UnionFind(N *N));
    construct(state);
    return state;
  }

  void construct(State state){
    UnionFind uf = state.uf;
    for (int n = 0;n < N;n++) {
      int i = rd.nextInt(pos.length);
      int j = rd.nextInt(pos.length);
      swap(pos,i,j);
    }

    for (var p:pos)
      if (uf.size(p) == 1)
        addCluster(p,state);
  }

  void swap(int[] arr,int i,int j){
    int t = arr[i];
    arr[i] = arr[j];
    arr[j] = t;
  }

  void addCluster(int p,State state){
    int[] teire = state.teire;
    UnionFind uf = state.uf;

    //品質の決定
    int shitu = A[p];
    teire[p] += shitu -A[p];
    state.teireN += shitu -A[p];
    //この値まで連結に使うコストを許可する
    int maxCost = Math.max(1,shitu *2 -10);
    List<Integer> list = new ArrayList<>();
    //{隣,隣を連結するコスト,コスト1以下で連結できる隣の隣のコストの総和,コスト1以下で連結できる隣の隣の数,隣の隣を連結するコストの最小値}
    //隣を連結するコストを重めに評価する
    Queue<int[]> que = new PriorityQueue<>(Comparator.comparing(t -> 6 *t[1] -1 *t[3] +1 *t[4]));
    //始点はちょっと違う方法で追加
    que.add(new int[]{p, 0, 0, 0});
    boolean flg = false;
    while (!que.isEmpty()) {

      var cur = que.poll();

      if (1 < uf.size(cur[0])) {
        //既に連結済みの区画の場合はスルー
        if (uf.root(cur[0]) == p)
          continue;

        if (flg)
          continue;

        uf.unite(cur[0],p);
        maxCost = 1;
        flg = true;
        continue;
      }
      //許容コスト超過の場合はスルー
      if (maxCost < cur[1])
        continue;

      //コストオーバーの場合はスルー
      if (M <= state.cost() +cur[1])
        continue;

      //品質が十分の場合コスト1までしか許可しない
      boolean kouritu = (cur[1] +cur[2]) *4 < shitu *(1 +cur[3]);
      if (shitu <= uf.size(p) && !kouritu)
        continue;

      //隣だけ連結
      uf.unite(p,cur[0]);
      list.add(cur[0]);
      teire[cur[0]] += cur[1];
      state.teireN += cur[1];
      //連結した区画の近傍をキューにいれる
      for (var s0:sur[cur[0]])
        if (A[s0] +state.teire[s0] == shitu && uf.root(s0) != uf.root(p) || uf.size(s0) == 1 && A[s0] <= shitu) {
          int[] arr = new int[5];
          arr[0] = s0;
          arr[1] = shitu -A[s0];
          arr[4] = 10;
          for (int i = 0;i < sur[s0].length;i++) {
            int s1 = sur[s0][i];
            if (uf.size(s1) == 1 && A[s1] <= shitu && s1 != p && shitu -A[s1] <= 1) {
              arr[2] += shitu -A[s1];
              arr[3]++;
              arr[4] = Math.min(arr[4],shitu -A[s1]);
            }
          }

          que.add(arr);
        }
    }
    if (shitu <= uf.size(p)) {
      //収穫可能なら始点を収穫位置に追加
      int scr = shitu *list.size();
      state.score += scr;
      if (uf.root(p) == p)
        state.shukaku.add(p);
      state.scr[uf.root(p)] += scr;
      //      assert uf.size(p) *shitu == state.scr[uf.root(p)];
    } else
      //そうでなければpと連結した箇所をリセットする
      for (int pp:list) {
        state.teireN -= teire[pp];
        teire[pp] = 0;
      }
  }

  /*
   * 収穫位置をちょっと変える
   * 収穫して得られる得点が少ないとこをn個変える
   */
  State nxt(State cur){
    State nxt = cur.copy();
    var shukaku = nxt.shukaku;

    //ランダムに破壊する
    int n = Math.min(seniN,shukaku.size());
    boolean[] hakai = new boolean[N *N];
    while (n-- > 0) {
      int rnd = revrnd(shukaku.size());
      Collections.swap(shukaku,rnd,shukaku.size() -1);
      int rem = shukaku.remove(shukaku.size() -1);
      hakai[rem] = true;
      nxt.score -= nxt.scr[rem];
      nxt.scr[rem] = 0;
    }

    var uf = nxt.uf;
    for (int pp = 0;pp < N *N;pp++)
      if (hakai[uf.root(pp)]) {
        nxt.teireN -= nxt.teire[pp];
        nxt.teire[pp] = 0;
        uf.par[pp] = pp;
        uf.size[pp] = 1;
        hakai[pp] = true;
      }
    construct(nxt);
    return nxt;
  }

  /*
   * 一連の操作を状態として持つ
   */
  static class State{
    //手入れ回数の合計
    int teireN;
    //どの区画に何回手入れを行うか
    int[] teire;
    //収穫する区画{位置,総和}
    List<Integer> shukaku;
    //スコア
    int score;
    //収穫位置のスコア
    int[] scr;
    //クラスタの管理
    UnionFind uf;

    State(int teireN,int[] teire,UnionFind uf){
      this.teireN = teireN;
      this.teire = teire;
      scr = new int[teire.length];
      shukaku = new ArrayList<>();
      this.uf = uf;
    }

    State copy(){
      State state = new State(teireN,Arrays.copyOf(teire,teire.length),uf.copy());

      boolean[] seen = new boolean[scr.length];
      state.scr = new int[scr.length];
      for (var s:shukaku) {
        int p = uf.root(s);
        if (seen[p])
          continue;
        state.scr[p] = uf.size(p) *(A[p] +teire[p]);
        state.score += state.scr[p];
        state.shukaku.add(p);
      }
      return state;
    }

    int cost(){ return teireN +shukaku.size(); }
  }

  /*
   *
   */
  static class Anneal{
    State best;
    State cur;
    int cnt;

    public Anneal(Main.State best){
      this.best = best;
      cur = best;
    }

  }

  /*
   * 形式を整えて出力
   */
  void out(State state){
    int[] cnt = new int[2];
    for (int p = 0;p < N *N;p++)
      for (int i = 0;i < state.teire[p] +baseTeire[p];i++) {
        out.println(new int[]{1, toi(p), toj(p)});
        cnt[0]++;
      }
    for (var p:state.shukaku) {
      if (cnt[0] +cnt[1] == M)
        return;
      out.println(new int[]{2, toi(p), toj(p)});
      cnt[1]++;
    }

    assert log(cnt,state);
  }

  boolean log(int[] cnt,State state){
    for (int i = 0;i < N;i++) {
      int[] arr = new int[N];
      for (int j = 0;j < N;j++) {
        int p = top(i,j);
        arr[j] = state.teire[p] +baseTeire[p];
      }
      log.println(arr);
    }
    log.println("");

    boolean[] shu = new boolean[N *N];
    for (var s:state.shukaku)
      shu[s] = true;
    for (int i = 0;i < N;i++) {
      int[] arr = new int[N];
      for (int j = 0;j < N;j++) {
        int p = top(i,j);
        if (!shu[state.uf.root(p)])
          arr[j] = A[p] +state.teire[p];
      }
      log.println(arr);
    }
    log.println(loop);
    log.println(cnt);
    log.println(state.score);
    log.println(calc(state));
    return true;
  }

  private int calc(State state){
    int ret = 0;

    int[] qual = new int[N *N];
    for (int i = 0;i < N *N;i++)
      qual[i] = A[i] +state.teire[i];

    UnionFind uf = new UnionFind(N *N);

    for (int p = 0;p < N *N;p++)
      for (var s:sur[p])
        if (qual[p] == qual[s])
          uf.unite(p,s);

    boolean[] seen = new boolean[N *N];

    for (var p:state.shukaku) {
      int pp = uf.root(p);
      if (!seen[pp])
        ret += uf.size(pp) *qual[pp];
      else
        log.println(null);
      seen[pp] = true;
    }

    return ret;
  }

  /* 以下ライブラリ系 */
  static class UnionFind{
    int[] par;
    int[] size;

    UnionFind(int[] size){
      par = new int[size.length];
      Arrays.setAll(par,i -> i);
      this.size = size;
    }

    UnionFind(int n){
      this(new int[n]);
      Arrays.fill(size,1);
    }

    UnionFind copy(){
      UnionFind uf = new UnionFind(size.length);
      System.arraycopy(par,0,uf.par,0,par.length);
      System.arraycopy(size,0,uf.size,0,par.length);
      return uf;
    }

    int root(int x){ return par[x] == x ? x : (par[x] = root(par[x])); }

    boolean same(int u,int v){ return root(u) == root(v); }

    boolean unite(int u,int v){
      if ((u = root(u)) == (v = root(v)))
        return false;

      par[v] = u;
      size[u] += size[v];
      return true;
    }

    int size(final int x){ return size[root(x)]; }

  }

  /*
   * [0,n)から乱択(右のほうが出やすい)
   */
  int rnd(int n){ return (int) Math.sqrt(rd.nextInt(n *n)); }

  /*
   * [0,n)から乱択(左のほうが出やすい)
   */
  int revrnd(int n){ return n -1 -(int) Math.sqrt(rd.nextInt(n *n)); }

  //一次元⇔二次元の座標変換
  int toi(int pos){ return pos /N; }

  int toj(int pos){ return pos %N; }

  int top(int i,int j){ return i *N +j; }

  /* 以下入出力と実行用メソッドのみ */

  /* 実行 */
  public static void main(String[] args){ new Main().exe(); }

  long elapsed(){ return System.currentTimeMillis() -st; }

  void exe(){
    assert local();
    out(solve());
    out.flush();
    log.println(elapsed());
  }

  /* 入力 */
  static class MyReader{
    byte[] buf = new byte[1 <<16];
    int head = 0;
    int tail = 0;
    InputStream in;

    public MyReader(InputStream in){ this.in = in; }

    byte read(){
      if (head == tail)
        try {
          tail = in.read(buf);
          head = 0;
        } catch (IOException e) {
          e.printStackTrace();
        }
      return buf[head++];
    }

    boolean isPrintable(byte c){ return 32 < c && c < 127; }

    boolean isNum(byte c){ return 47 < c && c < 58; }

    byte nextPrintable(){
      byte ret = read();
      while (!isPrintable(ret))
        ret = read();
      return ret;
    }

    int it(){ return (int) lg(); }

    int[] it(int N){
      int[] a = new int[N];
      Arrays.setAll(a,i -> it());
      return a;
    }

    int[][] it(int H,int W){ return arr(new int[H][],i -> it(W)); }

    int idx(){ return it() -1; }

    int[] idx(int N){
      int[] a = new int[N];
      Arrays.setAll(a,i -> idx());
      return a;
    }

    int[][] idx(int H,int W){ return arr(new int[H][],i -> idx(W)); }

    long lg(){
      byte i = nextPrintable();
      boolean negative = i == 45;
      long n = negative ? 0 : i -'0';
      while (isPrintable(i = read()))
        n = 10 *n +i -'0';
      return negative ? -n : n;
    }

    long[] lg(int N){
      long[] a = new long[N];
      Arrays.setAll(a,i -> lg());
      return a;
    }

    long[][] lg(int H,int W){ return arr(new long[H][],i -> lg(W)); }

    double dbl(){ return Double.parseDouble(str()); }

    double[] dbl(int N){
      double[] a = new double[N];
      Arrays.setAll(a,i -> dbl());
      return a;
    }

    double[][] dbl(int H,int W){ return arr(new double[H][],i -> dbl(W)); }

    char[] ch(){ return str().toCharArray(); }

    char[][] ch(int H){ return arr(new char[H][],i -> ch()); }

    String line(){
      StringBuilder sb = new StringBuilder();

      for (byte c;isPrintable(c = read()) || c == ' ';)
        sb.append((char) c);
      return sb.toString();
    }

    String str(){
      StringBuilder sb = new StringBuilder();
      sb.append((char) nextPrintable());

      for (byte c;isPrintable(c = read());)
        sb.append((char) c);
      return sb.toString();
    }

    String[] str(int N){ return arr(new String[N],i -> str()); }

    <T> T[] arr(T[] arr,IntFunction<T> f){
      Arrays.setAll(arr,f);
      return arr;
    }

    int[][] g(int N,int M,boolean d){
      List<List<Integer>> g = new ArrayList<>();
      for (int i = 0;i < N;i++)
        g.add(new ArrayList<>());

      for (int i = 0,u,v;i < M;i++) {
        g.get(u = idx()).add(v = idx());
        if (!d)
          g.get(v).add(u);
      }

      int[][] ret = new int[N][];
      for (int u = 0;u < N;u++) {
        ret[u] = new int[g.get(u).size()];
        for (int i = 0;i < ret[u].length;i++)
          ret[u][i] = g.get(u).get(i);
      }

      return ret;
    }
  }

  /* 出力 */
  static class MyWriter{
    OutputStream out;
    byte[] buf = new byte[1 <<16];
    byte[] ibuf = new byte[20];
    int tail = 0;

    public MyWriter(OutputStream out){ this.out = out; }

    void flush(){
      try {
        out.write(buf,0,tail);
        tail = 0;
      } catch (IOException e) {
        e.printStackTrace();
      }
    }

    void sp(){ write((byte) ' '); }

    void ln(){ write((byte) '\n'); }

    void write(byte b){
      buf[tail++] = b;
      if (tail == buf.length)
        flush();
    }

    void write(byte[] b,int off,int len){
      for (int i = off;i < off +len;i++)
        write(b[i]);
    }

    void write(long n){
      if (n < 0) {
        n = -n;
        write((byte) '-');
      }

      int i = ibuf.length;
      do {
        ibuf[--i] = (byte) (n %10 +'0');
        n /= 10;
      } while (n > 0);

      write(ibuf,i,ibuf.length -i);
    }

    void println(long n){
      write(n);
      ln();
    }

    void println(double d){ println(String.valueOf(d)); }

    void println(String s){ println(s.toCharArray()); }

    void println(char[] s){
      for (char b:s)
        write((byte) b);
      ln();
    }

    void println(int[] a){
      for (int i = 0;i < a.length;i++) {
        if (0 < i)
          sp();
        write(a[i]);
      }
      ln();
    }

    void println(long[] a){
      for (int i = 0;i < a.length;i++) {
        if (0 < i)
          sp();
        write(a[i]);
      }
      ln();
    }

    void println(double[] a){
      for (int i = 0;i < a.length;i++) {
        if (0 < i)
          sp();
        for (char b:String.valueOf(a[i]).toCharArray())
          write((byte) b);
      }
      ln();
    }

  }

  /* デバッグ用 */
  static class MyLogger{
    MyWriter log = new MyWriter(System.err){
      @Override
      void ln(){
        super.ln();
        flush();
      };
    };

    void println(Object obj){ assert write(obj); }

    boolean write(Object obj){
      if (obj instanceof char[])
        log.println((char[]) obj);
      else if (obj instanceof int[])
        log.println((int[]) obj);
      else if (obj instanceof long[])
        log.println((long[]) obj);
      else if (obj instanceof double[])
        log.println((double[]) obj);
      else
        log.println(Objects.toString(obj));
      return true;
    }
  }
}

