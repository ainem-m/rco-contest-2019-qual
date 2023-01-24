#![allow(non_snake_case, unused)]

use itertools::*;
use proconio::{marker::*, *};
use rand::prelude::*;
use rand::seq::SliceRandom;
use rand_pcg::Pcg64Mcg;
use std::{cmp::*, vec};
use std::{collections::*, fmt::format};

const N: usize = 50;
const M: usize = 2500;
const TL: f64 = 1.98;

/// Disjoint Set Union (Union Find)
#[derive(Clone)]
pub struct DSU {
    n: usize,
    // root node: -1 * component size
    // otherwise: parent
    parent_or_size: Vec<i32>,
}

impl DSU {
    /// Creates a new `Dsu`.
    ///
    /// # Constraints
    ///
    /// - $0 \leq n \leq 10^8$
    ///
    /// # Complexity
    ///
    /// - $O(n)$
    pub fn new(size: usize) -> Self {
        Self {
            n: size,
            parent_or_size: vec![-1; size],
        }
    }

    // `\textsc` does not work in KaTeX
    /// Performs the Uɴɪᴏɴ operation.
    ///
    /// # Constraints
    ///
    /// - $0 \leq a < n$
    /// - $0 \leq b < n$
    ///
    /// # Panics
    ///
    /// Panics if the above constraints are not satisfied.
    ///
    /// # Complexity
    ///
    /// - $O(\alpha(n))$ amortized
    pub fn merge(&mut self, a: usize, b: usize) -> usize {
        assert!(a < self.n);
        assert!(b < self.n);
        let (mut x, mut y) = (self.leader(a), self.leader(b));
        if x == y {
            return x;
        }
        if -self.parent_or_size[x] < -self.parent_or_size[y] {
            std::mem::swap(&mut x, &mut y);
        }
        self.parent_or_size[x] += self.parent_or_size[y];
        self.parent_or_size[y] = x as i32;
        x
    }

    /// Returns whether the vertices $a$ and $b$ are in the same connected component.
    ///
    /// # Constraints
    ///
    /// - $0 \leq a < n$
    /// - $0 \leq b < n$
    ///
    /// # Panics
    ///
    /// Panics if the above constraint is not satisfied.
    ///
    /// # Complexity
    ///
    /// - $O(\alpha(n))$ amortized
    pub fn same(&mut self, a: usize, b: usize) -> bool {
        assert!(a < self.n);
        assert!(b < self.n);
        self.leader(a) == self.leader(b)
    }

    /// Performs the Fɪɴᴅ operation.
    ///
    /// # Constraints
    ///
    /// - $0 \leq a < n$
    ///
    /// # Panics
    ///
    /// Panics if the above constraint is not satisfied.
    ///
    /// # Complexity
    ///
    /// - $O(\alpha(n))$ amortized
    pub fn leader(&mut self, a: usize) -> usize {
        assert!(a < self.n);
        if self.parent_or_size[a] < 0 {
            return a;
        }
        self.parent_or_size[a] = self.leader(self.parent_or_size[a] as usize) as i32;
        self.parent_or_size[a] as usize
    }

    /// Returns the size of the connected component that contains the vertex $a$.
    ///
    /// # Constraints
    ///
    /// - $0 \leq a < n$
    ///
    /// # Panics
    ///
    /// Panics if the above constraint is not satisfied.
    ///
    /// # Complexity
    ///
    /// - $O(\alpha(n))$ amortized
    pub fn size(&mut self, a: usize) -> usize {
        assert!(a < self.n);
        let x = self.leader(a);
        -self.parent_or_size[x] as usize
    }

    /// Divides the graph into connected components.
    ///
    /// The result may not be ordered.
    ///
    /// # Complexity
    ///
    /// - $O(n)$
    pub fn groups(&mut self) -> Vec<Vec<usize>> {
        let mut leader_buf = vec![0; self.n];
        let mut group_size = vec![0; self.n];
        for i in 0..self.n {
            leader_buf[i] = self.leader(i);
            group_size[leader_buf[i]] += 1;
        }
        let mut result = vec![Vec::new(); self.n];
        for i in 0..self.n {
            result[i].reserve(group_size[i]);
        }
        for i in 0..self.n {
            result[leader_buf[i]].push(i);
        }
        result
            .into_iter()
            .filter(|x| !x.is_empty())
            .collect::<Vec<Vec<usize>>>()
    }
}

pub fn get_time() -> f64 {
    static mut STIME: f64 = -1.0;
    let t = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .unwrap();
    let ms = t.as_secs() as f64 + t.subsec_nanos() as f64 * 1e-9;
    unsafe {
        if STIME < 0.0 {
            STIME = ms;
        }
        // ローカル環境とジャッジ環境の実行速度差はget_timeで吸収しておくと便利
        #[cfg(feature = "local")]
        {
            (ms - STIME) * 10.0
        }
        #[cfg(not(feature = "local"))]
        {
            (ms - STIME)
        }
    }
}

pub struct Input {
    a: Vec<usize>,
}
impl Input {
    fn new(a_org: &Vec<Vec<usize>>) -> Self {
        // 入力の盤面より計算効率のいい1次元の場面に変換する
        let mut a: Vec<usize> = vec![0; N * N];
        for i in 0..N {
            for j in 0..N {
                a[i * N + j] = a_org[i][j];
            }
        }
        Self { a }
    }
}

#[derive(Clone)]
// union findは，例えば6を9まで育てたあと，それを9のunionにマージするのは容易だが
// 6を元に所属していた6のunionから切り離すのは難しい
// そこで，9まで育てるのは8,7まで，6まで育てるのは5,4までというように，お互いに不干渉にする
pub struct State {
    end_turn: usize,
    turn: usize,
    pub a: Vec<usize>,
    pub is_harvested: Vec<bool>,
    pub uf: DSU,              // 9,6,3などのunion find
    pub target: usize,        // 上記をインデックスする変数
    pub is_unused: Vec<bool>, // 7,8などを9に手入れしたが使わなかった物の位置
    pub game_score: usize,
    pub evaluated_score: usize,
    pub output: Vec<String>,
}
impl State {
    pub fn new(input: &Input, target: usize) -> Self {
        let mut uf = DSU::new(N * N);
        for i in 0..N * N {
            // 品質targetのunion find
            if i / N < N - 1 && input.a[i + N] == target && input.a[i] == target {
                // 下の要素が自分と同じなら併合する
                uf.merge(i, i + N);
            }
            if i % N < N - 1 && input.a[i + 1] == target && input.a[i] == target {
                uf.merge(i, i + 1);
            }
        }
        Self {
            end_turn: M,
            turn: 0,
            a: input.a.clone(),
            is_harvested: vec![false; N * N],
            uf,
            target,
            is_unused: vec![false; N * N],
            game_score: 0,
            evaluated_score: 0,
            output: vec![],
        }
    }
    // 盤面の評価値
    pub fn evaluated_score(&mut self) {
        self.evaluated_score = self.game_score;
    }
    // 終了判定
    pub fn is_done(&self) -> bool {
        self.turn == self.end_turn
    }
    // 品質がtargetのunionを作る
    pub fn make_union_of_target(&mut self, target: usize) {
        let mut uf = DSU::new(N * N);
        for i in 0..N * N {
            if self.a[i] == target {
                // 下を見るに相当
                if i / N < N - 1 && self.a[i + N] == target {
                    uf.merge(i, i + N);
                }
                // 右を見るに相当
                if i % N < N - 1 && self.a[i + 1] == target {
                    uf.merge(i, i + 1);
                }
            }
        }
        self.uf = uf;
        self.target = target;
    }
    // 品質zの隣りにあるような品質x~yの野菜で，手入れ可能なものの位置を，手入れ後のzのunionのサイズが大きい順に優先度付きキューで取得する
    pub fn get_pos_of_can_maintain_veg_from_x_to_y_next_to_z(
        &mut self,
        x: usize,
        y: usize,
        z: usize,
    ) -> BinaryHeap<(usize, usize)> {
        let mut pos_x_to_y_bh = BinaryHeap::new();
        for i in 0..N * N {
            // iを手入れしてできるunionのサイズ
            let mut tmp_size = 0;
            // 上下左右いくつのunionと繋がることになるか
            let mut num_union = 0;
            // 上下左右ですでに見たunionは見たくないので，すでに見たunionの親を保持する
            let mut seen = HashSet::new();
            // 自身がx~y, 親が未収穫
            if x <= self.a[i] && self.a[i] <= y && !self.is_harvested[self.uf.leader(i)] {
                // 上がz，上の親をまだ見ていない，上の親が未収穫
                if 0 < i / N
                    && self.a[i - N] == z
                    && !seen.contains(&self.uf.leader(i - N))
                    && !self.is_harvested[self.uf.leader(i - N)]
                {
                    // 上のサイズを足す
                    tmp_size += self.uf.size(i - N);
                    num_union += 1;
                    seen.insert(self.uf.leader(i - N));
                }
                // 下
                if i / N < N - 1
                    && self.a[i + N] == z
                    && !seen.contains(&self.uf.leader(i + N))
                    && !self.is_harvested[self.uf.leader(i + N)]
                {
                    tmp_size += self.uf.size(i + N);
                    num_union += 1;
                    seen.insert(self.uf.leader(i + N));
                }
                // 左
                if 0 < i % N
                    && self.a[i - 1] == z
                    && !seen.contains(&self.uf.leader(i - 1))
                    && !self.is_harvested[self.uf.leader(i - 1)]
                {
                    tmp_size += self.uf.size(i - 1);
                    num_union += 1;
                    seen.insert(self.uf.leader(i - 1));
                }
                // 右
                if i % N < N - 1
                    && self.a[i + 1] == z
                    && !seen.contains(&self.uf.leader(i + 1))
                    && !self.is_harvested[self.uf.leader(i + 1)]
                {
                    tmp_size += self.uf.size(i + 1);
                    num_union += 1;
                    seen.insert(self.uf.leader(i + 1));
                }
                // 上下左右いずれかにzのunionがあり，合計サイズがz未満か，自身の価値がある程度高いか，上下左右複数のunionを繋ぐなら候補に入れる
                if 0 < tmp_size && (tmp_size < z || 4 <= self.a[i] || num_union > 1) {
                    // 合計サイズ，手入れにかかるコスト，複数のunionをつなぐことを加味して評価値を決め，優先度付きキューに入れる
                    let value_i = (tmp_size * 2).saturating_sub(z - self.a[i]) + num_union;
                    pos_x_to_y_bh.push((value_i, i));
                }
            }
        }
        pos_x_to_y_bh
    }
    // 指定の野菜をxまで手入れする
    pub fn maintain_to_x(&mut self, pos: usize, x: usize) {
        while self.a[pos] < x {
            self.a[pos] += 1;
            self.turn += 1;
            self.output.push(format!("1 {} {}", pos / N, pos % N));
            if self.is_done() {
                return;
            }
        }
        // 現在のtargetのunion findを更新する
        // 上
        if 0 < pos / N && self.a[pos - N] == x && !self.is_harvested[self.uf.leader(pos - N)] {
            self.uf.merge(pos, pos - N);
        }
        // 下
        if pos / N < N - 1 && self.a[pos + N] == x && !self.is_harvested[self.uf.leader(pos + N)] {
            self.uf.merge(pos, pos + N);
        }
        // 左
        if 0 < pos % N && self.a[pos - 1] == x && !self.is_harvested[self.uf.leader(pos - 1)] {
            self.uf.merge(pos, pos - 1);
        }
        // 右
        if pos % N < N - 1 && self.a[pos + 1] == x && !self.is_harvested[self.uf.leader(pos + 1)] {
            self.uf.merge(pos, pos + 1);
        }
    }
    // 指定の品質xの野菜をすべてyまで手入れする
    // ただしunusedフラグが付与されているものは使わない
    pub fn maintain_all_x_to_y(&mut self, x: usize, y: usize) {
        for i in 0..N * N {
            if self.is_unused[i] {
                continue;
            }
            if self.a[i] == x {
                while self.a[i] < y {
                    self.a[i] += 1;
                    self.turn += 1;
                    self.output.push(format!("1 {} {}", i / N, i % N));
                    if self.is_done() {
                        return;
                    }
                }
                // 現在のtargetのunion findを更新する
                // 上
                if 0 < i / N && self.a[i - N] == y && !self.is_harvested[self.uf.leader(i - N)] {
                    self.uf.merge(i, i - N);
                }
                // 下
                if i / N < N - 1 && self.a[i + N] == y && !self.is_harvested[self.uf.leader(i + N)]
                {
                    self.uf.merge(i, i + N);
                }
                // 左
                if 0 < i % N && self.a[i - 1] == y && !self.is_harvested[self.uf.leader(i - 1)] {
                    self.uf.merge(i, i - 1);
                }
                // 右
                if i % N < N - 1 && self.a[i + 1] == y && !self.is_harvested[self.uf.leader(i + 1)]
                {
                    self.uf.merge(i, i + 1);
                }
            }
        }
    }
    // 品質xの野菜で収穫可能なものを，サイズが大きい順に優先度付きキューで取得する
    // (x, self.target) = (9, 0), (5, 1), (3, 2)などであってほしい
    pub fn get_pos_of_can_harvest_veg_of_x(&mut self, x: usize) -> BinaryHeap<(usize, usize)> {
        let mut pos_x_bh = BinaryHeap::new();
        for i in 0..N * N {
            // 自身がx，親が未収穫，自身を含むunionのサイズがx以上
            if self.a[i] == x && !self.is_harvested[self.uf.leader(i)] && self.uf.size(i) >= x {
                pos_x_bh.push((self.uf.size(i), i));
            }
        }
        pos_x_bh
    }
    // 指定の野菜を収穫する
    pub fn harvest(&mut self, pos: usize) {
        self.turn += 1;
        self.is_harvested[self.uf.leader(pos)] = true;
        // 収穫場所の品質*収穫場所を含むunionのサイズの得点を得る
        self.game_score += self.a[pos] * self.uf.size(pos);
        self.output.push(format!("2 {} {}", pos / N, pos % N));
        if self.is_done() {
            return;
        }
    }
    // 7,8などから9まで育てたが使わなかったものを記録する
    // targetが9から切り替わる前に使用する (そうでないとunion findを更新してしまう)
    pub fn memorize_unused_x(&mut self, a_org: &Vec<Vec<usize>>, x_list: &Vec<usize>) {
        for i in 0..N * N {
            // 元が7などで，9まで手入れされていて，収穫済みでないもの
            for &x in x_list {
                if a_org[i / N][i % N] == x
                    && self.a[i] == 9
                    && !self.is_harvested[self.uf.leader(i)]
                {
                    self.is_unused[i] = true;
                }
            }
        }
    }
    pub fn memorize_all_unused(&mut self, a_org: &Vec<Vec<usize>>) {
        for i in 0..N * N {
            // 元が7などで，9まで手入れされていて，収穫済みでないもの
            if a_org[i / N][i % N] < self.a[i] && !self.is_harvested[self.uf.leader(i)] {
                self.is_unused[i] = true;
            }
        }
    }
}

fn main() {
    get_time();
    input! {
        n_: usize,
        m_: usize,
        a_org: [[usize; N]; N],
    }
    let mut input = Input::new(&a_org);
    let mut state_org = State::new(&input, 9);

    // 9がtargetの最低保証turn
    let target_9_turn = 1500;
    // 7,8を9まで育てて6以下は周りからつなげて育てる
    let mut target_list = vec![9, 6, 5, 4, 3, 2, 1];
    let grow_list = vec![8, 7];

    // TLまで乱数ふってループ
    let mut rng = rand_pcg::Pcg64Mcg::new(42);
    let mut best_output = vec![];
    let mut best_score = 0;
    let mut iter = 0;
    while get_time() < TL {
        iter += 1;
        // このイテレーションで使うstateをオリジナルからcloneしてくる
        let mut state = state_org.clone();
        // 7,8を9まですべて育てる
        // 2iter以降では，前回までで使われなかったものを優先度下げるようになっている
        for &g in &grow_list {
            state.maintain_all_x_to_y(g, 9);
        }
        // targetとするturn数
        let mut target_turn = vec![];
        let mut prev_turn = target_9_turn;
        for i in 0..target_list.len() - 1 {
            let turn = rng.gen_range(
                std::cmp::min(M, prev_turn + 300 / (i + 1)),
                std::cmp::min(M + 1, prev_turn + 500 / (i + 1)),
            );
            target_turn.push(turn);
            prev_turn = turn;
        }
        target_turn.push(M);
        // 上記をインデックスする変数
        let mut target_index = 0;
        // targetとするturnのうち，どれだけ経過したらharvestするかの割合を格納
        // let harvest_turn_ratio = vec![0.9, 0.9, 0.95, 0.95, 0.98, 0.98, 0.99];

        while get_time() < TL && !state.is_done() {
            // 後半に差し掛かったら収穫可能なtargetを探し，あれば収穫する
            if state.turn as f64 > target_turn[target_index] as f64 * 0.91 {
                let mut pos_of_can_harvest_target =
                    state.get_pos_of_can_harvest_veg_of_x(target_list[target_index]);
                if !pos_of_can_harvest_target.is_empty() {
                    let action_tuple = pos_of_can_harvest_target.pop().unwrap();
                    state.harvest(action_tuple.1);
                    // 収穫後，もしtargetを収穫するturnを過ぎていたらtargetを変える
                    if state.turn > target_turn[target_index] {
                        // targetが9なら，ここまでで手入れしたが使用しなかった7,8をもとのstate_orgへ保存する
                        if target_list[target_index] == 9 {
                            state.memorize_unused_x(&a_org, &grow_list);
                            state_org.is_unused = state.is_unused.clone();
                        }
                        target_index += 1;
                        state.make_union_of_target(target_list[target_index]);
                    }
                    continue;
                }
            }

            // 最後は絶対収穫させる
            if state.turn == M - 1 {
                continue;
            }

            // targetの隣りにある収穫可能な野菜を，収穫後のunionのサイズが大きい順に取得する
            let mut pos_x = state.get_pos_of_can_maintain_veg_from_x_to_y_next_to_z(
                1,
                target_list[target_index] - 1,
                target_list[target_index],
            );
            if !pos_x.is_empty() {
                let action_tuple = pos_x.pop().unwrap();
                state.maintain_to_x(action_tuple.1, target_list[target_index]);
                continue;
            }
        }

        // このイテレーションでのスコアを取得し，ベストを更新していればupdateする
        if state.game_score > best_score {
            best_score = state.game_score;
            best_output = state.output.clone();
            eprintln!("update best score: {}", best_score);
        }

        // イテレーションが終わったら使わなかったやつを記録し，次回は使わない
        state.memorize_all_unused(&a_org);
    }

    /// [結果発表]
    eprintln!("score: {}", best_score);
    eprintln!("time: {:.3}", get_time());
    eprintln!("iter: {}", iter);
    for o in &best_output {
        println!("{}", o);
    }
}
