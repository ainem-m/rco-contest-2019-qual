#![allow(dead_code)]
#![allow(unused)]
use core::fmt::Debug;
use fixedbitset::FixedBitSet;
use num_traits::Pow;
use rand::prelude::ThreadRng;
use rand::seq::SliceRandom;
use rand::{thread_rng, Rng};
use rand_pcg::Pcg64Mcg;
use std::cmp::Ordering;
use std::collections::BTreeMap;
use std::collections::BTreeSet;
use std::collections::BinaryHeap;
use std::collections::VecDeque;
use std::fmt::Display;
use std::iter::FromIterator;
use std::ops::Add;
use std::ops::Sub;
use std::str::FromStr;
use std::time::{Duration, Instant};
use std::{env, vec};

use proconio::input;
use proconio::marker::{Bytes, Chars};
use std::fmt;
use std::ops::Bound::*;
use std::ops::RangeBounds;

const INF: usize = 100000000;

type Pos = (usize, usize);

#[derive(Clone, Eq, PartialEq, PartialOrd, Ord, Debug)]
struct Area {
    init_quality: usize,
    quality: usize,
    cut: bool,
    growed: bool,
}

#[derive(Clone, Eq, PartialEq, PartialOrd, Ord, Debug)]
struct Farm {
    size: usize,
    area: Vec<Area>,
}

impl Farm {
    fn hash(&self, pos: &Pos) -> usize {
        let (i, j) = *pos;
        return i * self.size + j;
    }
    fn unhash(&self, hash: usize) -> Pos {
        let i = hash / self.size;
        let j = hash % self.size;
        return (i, j);
    }
    fn get_adj(&self, pos: &Pos) -> Vec<Pos> {
        let mut ans = vec![];
        if pos.0 > 0 {
            ans.push((pos.0 - 1, pos.1));
        }
        if pos.0 < self.size - 1 {
            ans.push((pos.0 + 1, pos.1));
        }
        if pos.1 > 0 {
            ans.push((pos.0, pos.1 - 1));
        }
        if pos.1 < self.size - 1 {
            ans.push((pos.0, pos.1 + 1));
        }
        return ans;
    }
    fn get_connected(&self, start: &Pos, used: &mut Vec<bool>) -> Vec<Pos> {
        let mut ans = vec![];
        let h = self.hash(start);
        if used[h] {
            return ans;
        }
        let v = self.get_quality(start);

        let mut poses = vec![start.clone()];
        while let Some(pos) = poses.pop() {
            if used[self.hash(&pos)] {
                continue;
            }
            used[self.hash(&pos)] = true;
            for next_pos in self.get_adj(&pos) {
                let next_hash = self.hash(&next_pos);
                if used[next_hash] {
                    continue;
                }
                if self.get_quality(&next_pos) != v {
                    continue;
                }
                poses.push(next_pos);
            }
            ans.push(pos);
        }
        return ans;
    }
    fn enum_cons(&self) -> Vec<Vec<Pos>> {
        let mut used = vec![false; self.size * self.size];
        let mut ans = vec![];
        for i in 0..self.size {
            for j in 0..self.size {
                let start = (i, j);
                if used[self.hash(&start)] {
                    continue;
                }
                let cons = self.get_connected(&start, &mut used);
                ans.push(cons)
            }
        }
        return ans;
    }

    fn eval_score(&self) -> usize {
        let mut score = 0;
        for i in 0..self.size {
            for j in 0..self.size {
                let pos = (i, j);
                if self.is_cut(&pos) {
                    score += self.get_quality(&pos);
                }
            }
        }
        return score;
    }

    fn get_quality(&self, pos: &Pos) -> usize {
        let h = self.hash(pos);
        return self.area[h].quality;
    }

    fn set_quality(&mut self, pos: &Pos, quality: usize) {
        let h = self.hash(pos);
        self.area[h].quality = quality
    }

    fn reset_grow(&mut self, pos: &Pos) {
        let h = self.hash(pos);
        self.area[h].quality = self.area[h].init_quality;
        self.area[h].growed = false;
        self.area[h].cut = false;
    }

    fn is_cut(&self, pos: &Pos) -> bool {
        let h = self.hash(pos);
        return self.area[h].cut;
    }

    fn set_cut(&mut self, pos: &Pos, status: bool) {
        let h = self.hash(pos);
        self.area[h].cut = status
    }

    fn is_growed(&self, pos: &Pos) -> bool {
        let h = self.hash(pos);
        return self.area[h].growed;
    }
    fn get_growth(&self, pos: &Pos) -> usize {
        let h = self.hash(pos);
        return self.area[h].quality - self.area[h].init_quality;
    }

    fn set_growed(&mut self, pos: &Pos, status: bool) {
        let h = self.hash(pos);
        self.area[h].growed = status
    }

    fn from_vec(v: &Vec<Vec<usize>>) -> Self {
        let size = v.len();
        let mut area = vec![
            Area {
                init_quality: 0,
                quality: 0,
                growed: false,
                cut: false,
            };
            size * size
        ];
        let mut farm = Farm { size, area };
        for i in 0..size {
            for j in 0..size {
                let pos = (i, j);
                let h = farm.hash(&pos);
                let quality = v[i][j];
                farm.area[h] = Area {
                    init_quality: quality,
                    quality: quality,
                    cut: false,
                    growed: false,
                };
            }
        }
        return farm;
    }
}

struct Input {
    n: usize,
    m: usize,
    a: Vec<Vec<usize>>,
}

impl Input {
    fn read_init() -> Self {
        input! {
            n: usize,
            m: usize,
            a: [[usize; n]; n]
        }
        return Self { n, m, a };
    }
}

fn collect(
    start: Pos,
    farm: &mut Farm,
    rest_act: usize,
    borders: &Vec<usize>,
    cospa_border: f64,
) -> usize {
    let cluster_value = farm.get_quality(&start);
    let mut candidates = BinaryHeap::new();
    candidates.push((INF, start));
    let mut total_cost = 1;
    let mut gain_score = 0;
    let mut grows = vec![];
    let mut need_cut = true;

    while let Some((_, pos)) = candidates.pop() {
        if farm.is_growed(&pos) {
            continue;
        }
        let target_value = farm.get_quality(&pos);
        let cost = cluster_value - target_value;
        let border = borders[cluster_value];
        if cost > border {
            break;
        }
        if total_cost + cost > rest_act {
            break;
        }
        gain_score += target_value;
        total_cost += cost;
        farm.set_growed(&pos, true);
        farm.set_quality(&pos, cluster_value);
        grows.push((pos, cost));

        let nexts = farm.get_adj(&pos);
        for next in nexts {
            let is_growed = farm.is_growed(&next);
            if is_growed {
                continue;
            }
            let is_cut = farm.is_cut(&next);
            let next_value = farm.get_quality(&next);
            if is_cut {
                if next_value == cluster_value {
                    need_cut = false;
                    total_cost -= 1;
                }

                continue;
            }

            if next_value <= cluster_value {
                let cost = cluster_value - next_value;
                candidates.push((INF - cost, next))
            }
        }
    }
    let can_not_cut = need_cut && grows.len() < cluster_value;
    let bad_cospa = (gain_score as f64 / total_cost as f64) < cospa_border;
    if can_not_cut || bad_cospa {
        for (pos, _) in &grows {
            farm.reset_grow(pos);
        }
        return 0;
    }

    for (pos, _) in &grows {
        farm.set_cut(pos, true)
    }

    return total_cost;
}

fn print_result(farm: &Farm) {
    let cons = farm.enum_cons();
    let mut grows = vec![];
    let mut cuts = vec![];
    for group in cons {
        let start = group[0];
        if farm.is_cut(&start) {
            for pos in group {
                for _ in 0..farm.get_growth(&pos) {
                    grows.push(pos);
                }
            }
            cuts.push(start)
        }
    }
    for pos in grows {
        println!("1 {} {}", pos.0, pos.1)
    }
    for pos in cuts {
        println!("2 {} {}", pos.0, pos.1)
    }
}

fn get_initial_search_order(farm: &Farm, group: &Vec<Pos>, order: &Vec<usize>) -> usize {
    let quality = farm.get_quality(&group[0]);
    return order[quality];
}

fn initial_search(
    input: &Input,
    borders: &Vec<usize>,
    order: &Vec<usize>,
    cospa_border: f64,
) -> (Farm, usize) {
    let mut farm = Farm::from_vec(&input.a);
    let mut cons = farm.enum_cons();
    cons.sort_by_key(|x| get_initial_search_order(&farm, x, order));

    let mut rest_act = input.m;
    for target in cons {
        let start = target[0];
        let this_cost = collect(start, &mut farm, rest_act, borders, cospa_border);

        if rest_act >= this_cost {
            rest_act -= this_cost;
        }
    }
    return (farm, rest_act);
}

fn collect_unused_9(farm: &Farm) -> Vec<Pos> {
    let mut ans = vec![];
    for i in 0..farm.size {
        for j in 0..farm.size {
            let pos = (i, j);
            if farm.get_quality(&pos) == 9 && !farm.is_cut(&pos) {
                ans.push(pos)
            }
        }
    }
    return ans;
}

fn collect_bridge4(farm: &Farm) -> Vec<Pos> {
    let mut ans = vec![];
    for i in 0..farm.size {
        for j in 0..farm.size {
            let pos = (i, j);
            if farm.get_quality(&pos) < 6 && !farm.is_cut(&pos) {
                let mut has_cut_9 = false;
                let mut has_not_cut_9 = false;
                for next in farm.get_adj(&pos) {
                    if farm.get_quality(&next) == 9 {
                        if farm.is_cut(&next) {
                            has_cut_9 = true
                        }
                    }
                    if farm.get_quality(&next) >= 8 {
                        if !farm.is_cut(&next) {
                            has_not_cut_9 = true
                        }
                    }
                }
                if has_cut_9 && has_not_cut_9 {
                    ans.push(pos)
                }
            }
        }
    }
    return ans;
}

// startはcut済み9に隣接想定
fn calc_extend9_cost(start: &Pos, farm: &mut Farm, save: bool, border: usize) -> (usize, usize) {
    let cluster_value = 9;
    let mut candidates = BinaryHeap::new();
    candidates.push((INF, *start));
    let mut total_cost = 0;
    let mut gain_score = 0;
    let mut grows = vec![];

    while let Some((_, pos)) = candidates.pop() {
        if farm.is_growed(&pos) {
            continue;
        }
        let target_value = farm.get_quality(&pos);
        let cost = cluster_value - target_value;
        if pos != *start {
            if cost > border {
                break;
            }
        }

        gain_score += target_value;
        total_cost += cost;
        farm.set_growed(&pos, true);
        farm.set_quality(&pos, cluster_value);
        grows.push((pos, cost));

        let nexts = farm.get_adj(&pos);
        for next in nexts {
            let is_growed = farm.is_growed(&next);
            if is_growed {
                continue;
            }
            let is_cut = farm.is_cut(&next);
            let next_value = farm.get_quality(&next);
            if is_cut {
                continue;
            }

            if next_value <= cluster_value {
                let cost = cluster_value - next_value;
                candidates.push((INF - cost, next))
            }
        }
    }
    if !save {
        for (pos, _) in &grows {
            farm.reset_grow(pos);
        }
    } else {
        for (pos, _) in &grows {
            farm.set_cut(pos, true)
        }
    }
    return (gain_score, total_cost);
}

fn get_bridges_order_by_cospa(farm: &mut Farm, border: usize) -> Vec<(usize, usize, Pos)> {
    let bridges = collect_bridge4(&farm);
    let mut bridges_with_cospa = vec![];
    for start in bridges {
        let (gain_score, total_cost) = calc_extend9_cost(&start, farm, false, border);

        let cospa = if total_cost == 0 {
            INF
        } else {
            gain_score * 100 / total_cost
        };
        bridges_with_cospa.push((cospa, gain_score, total_cost, start))
    }
    bridges_with_cospa.sort_by_key(|x| INF - x.0);
    return bridges_with_cospa.iter().map(|x| (x.1, x.2, x.3)).collect();
}

fn search(farm: &mut Farm, rest_act: usize, border: usize) {
    let mut rest_act = rest_act;

    let bridges = get_bridges_order_by_cospa(farm, border);
    for (score, cost, pos) in bridges {
        if cost > rest_act {
            continue;
        }
        if score * 10 / cost < 20 {
            continue;
        }
        let (score, cost2) = calc_extend9_cost(&pos, farm, false, border);
        if cost2 != cost {
            continue;
        }

        calc_extend9_cost(&pos, farm, true, border);

        rest_act -= cost2;
    }
    let bridges = get_bridges_order_by_cospa(farm, border);

    for (score, cost, pos) in bridges {
        if cost > rest_act {
            continue;
        }
        if score * 10 / cost < 5 {
            continue;
        }
        let (score, cost2) = calc_extend9_cost(&pos, farm, true, border);

        rest_act -= cost2;
    }

    consume_rest(farm, rest_act)
}

fn calc_cost(farm: &Farm) -> usize {
    let cons = farm.enum_cons();
    let mut grows = 0;
    let mut cuts = 0;
    for group in cons {
        let start = group[0];
        if farm.is_cut(&start) {
            for pos in group {
                for _ in 0..farm.get_growth(&pos) {
                    grows += 1;
                }
            }
            cuts += 1;
        }
    }
    return grows + cuts;
}

fn consume_rest(farm: &mut Farm, rest: usize) {
    let mut rest = rest;
    let mut candidates = vec![];
    for i in 0..farm.size {
        for j in 0..farm.size {
            let pos = (i, j);
            if farm.is_cut(&pos) {
                continue;
            }

            let quality = farm.get_quality(&pos);
            if rest == 1 && quality == 1 {
                farm.set_cut(&pos, true);
                return;
            }
            for n in farm.get_adj(&pos) {
                if !farm.is_cut(&n) {
                    continue;
                }
                let n_quality = farm.get_quality(&n);
                if n_quality <= quality {
                    continue;
                }
                let cost = n_quality - quality;
                if cost > rest {
                    continue;
                }
                let cospa = n_quality * 10 / cost;
                candidates.push((cospa, cost, n_quality, quality, pos));
            }
        }
    }
    candidates.sort_by_key(|x| INF - x.0);
    for (cospa, cost, n_quality, quality, pos) in candidates {
        if farm.is_cut(&pos) || cost > rest {
            continue;
        }
        farm.set_quality(&pos, n_quality);
        farm.set_cut(&pos, true);
        rest -= n_quality - quality;
        if rest == 0 {
            return;
        }
    }
}

fn main() {
    let input = Input::read_init();
    let mut best_farm = Farm::from_vec(&input.a);
    let mut best_score = best_farm.eval_score();
    let mut start_time = Instant::now();

    let borders_list = vec![
        vec![0, 1, 1, 1, 1, 2, 2, 2, 4, 4],
        vec![0, 1, 1, 1, 2, 2, 2, 2, 4, 4],
        vec![0, 1, 1, 1, 2, 2, 2, 3, 4, 4],
        vec![0, 1, 1, 1, 2, 2, 3, 3, 4, 4],
        vec![0, 1, 1, 1, 2, 2, 3, 4, 4, 4],
        vec![0, 1, 1, 1, 2, 3, 3, 4, 4, 4],
    ];
    let mut head = vec![1, 2, 3, 4];
    let mut tail = vec![5, 6, 7, 8];
    let mut rng = thread_rng();
    while Instant::now() - start_time < Duration::from_millis(1970) {
        let mut order = vec![INF, 9];
        head.shuffle(&mut rng);
        tail.shuffle(&mut rng);
        for v in &head {
            order.push(*v)
        }
        for v in &tail {
            order.push(*v)
        }
        let cospa_border = rng.gen_range(20, 30) as f64 / 10.0;
        for borders in &borders_list {
            let (mut farm, rest_act) = initial_search(&input, &borders, &order, cospa_border);
            search(&mut farm, rest_act, borders[9]);
            let score = farm.eval_score();
            if best_score < score {
                best_farm = farm;
                best_score = score;
                eprintln!("{} {:?} {:?} {}", best_score, borders, order, cospa_border)
            }
        }
    }
    eprintln!("{} {:?}", best_score, Instant::now() - start_time);
    print_result(&best_farm);
}

#[test]
fn test() {
    let size = 5;
    let farm = Farm::from_vec(&vec![
        vec![1, 2, 3, 4, 5],
        vec![1, 2, 2, 2, 6],
        vec![3, 2, 5, 2, 7],
        vec![3, 2, 5, 2, 7],
        vec![3, 9, 5, 2, 2],
    ]);
    let mut used = vec![false; 25];
    let start = (4, 4);
    let cons = farm.get_connected(&start, &mut used);
    let mut print = vec![vec![0; size]; size];
    for p in cons {
        print[p.0][p.1] = 1
    }
    for l in print {
        eprintln!("{:?}", l)
    }
}
