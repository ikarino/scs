// Copyright 2014 ikarino
#include <random>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <iostream>
#include "unit.h"
#include "status.h"
#include "map.h"

int Enemy::deadSG_ = 0;
int Enemy::directionElse_ = 0;
int Enemy::directionAll_ = 0;

/*
 *
 * モンスター全般について扱っていく
 * 変更の可能性があるとすれば
 *
 *  -(1) Fellow::Action
 *  -(2) Fellow::特技
 *  -(2) Enemy::Action
 *  -(3) Direction
 * 
 * (1)(2)は特技追加時のモンスターの種類にあわせた修正。特技使用率は(1)で定義していく。
 * (3)は敵スモグルが仲間スモグルを殴ったときに分裂方向について。
 *    今のところランダムだけど、面倒だからこのままでも良さそう。
 * (4)は分裂方向。不完全。
 *
 *
 */

using std::vector;


void Unit::Lose25() {
  if (mtrand()%100 < 92) {
    hp_-=25;
  }
};

void Fellow::Action(ENEMYVECTOR *u_enemy, FELLOWVECTOR *u_fellow, Map *mp) {
  if (seal_) {  // 封印状態なので通常攻撃
    Attack(u_enemy, mp);
  } else {  // モンスターにあわせて特技使用or通常攻撃
    switch (sp_) {
    case 120:  // ホイミスライム
      // ここでホイミと攻撃を分岐させる必要は無い。
      // 特技使用率やフラフラ行動はHoimi関数内部に含まれてる。
      Hoimi(u_enemy, u_fellow, mp);  
      break;
    case 121:  // メイジももんじゃ
      if (mtrand()%10 < 1) {
	Kuchifuji(u_enemy, mp);
      } else {
	Attack(u_enemy, mp);
      }
      break;
    case 606:  // おばけキノコ
      if (mtrand()%20 < 3) {  // 2014.5.28 15% wiki出典
	DokuNoIki(u_enemy, mp);
      } else {
	Attack(u_enemy, mp);
      }
      break;
    case 661:  // ランガー
    case 647:  // フライングデビル
      if (mtrand()%10 < 3) {
	MoonSault(u_enemy, mp);
      } else {
	Attack(u_enemy, mp);
      }
      break;
    case 318:  // スライムブレス
    case 324:  // ドラゴスライム
      if (mtrand()%10 < 3) {
	Fire(u_enemy, mp, 10);
      } else {
	Attack(u_enemy, mp);
      }
      break;
    case 325:  // ドラゴメタル
      if (mtrand()%10 < 3) {
	Fire(u_enemy, mp, 20);
      } else {
	Attack(u_enemy, mp);
      }
      break;
    case 802:  // オニオーン
    case 805:  // たまねぎマン
      break;
    default:
      Attack(u_enemy, mp);
      break;
    }
  }
  hp_ += static_cast<double>(mhp_)/rec_;
  if (hp_>mhp_) hp_ = mhp_;
}




Fellow::Fellow() {}

Fellow& Fellow::operator=(const Fellow& fl) {
  // unit
  sp_ = fl.sp_;
  mhp_ = fl.mhp_;
  at_ = fl.at_;
  df_ = fl.df_;
  seal_ = fl.seal_;
  place_ = fl.place_;
  // fellow only
  id_ = fl.id_;
  lv_ = fl.lv_;
  dp_ = fl.dp_;
  atW_ = fl.atW_;
  dfW_ = fl.dfW_;
  speed_ = fl.speed_;
  rec_ = fl.rec_;
  exp_ = fl.exp_;
  totalEXP_ = fl.totalEXP_;
  return *this;
}

/*! @brief 敵に行動をさせる関数。
  @param[in] u_enemy  敵ベクトル
  @param[in] u_fellow 味方ベクトル
  @param[in] mp       マップ
  @param[in] dead     仲間が死んだときに、その仲間の情報を代入する
*/
void Enemy::Action(ENEMYVECTOR *u_enemy,
                   FELLOWVECTOR *u_fellow, Map *mp, Fellow *dead) {
  // まずは攻撃可能位置に敵がいるか確認
  bool attackable[8];
  for (int i = 0; i < 8; i++) {
    attackable[i] = true;
  }
  // 壁で遮られて攻撃不可能になる場所
  if (mp->GetMP(place_-1) == 1) {
    attackable[0] = false;
    attackable[5] = false;
  }
  if (mp->GetMP(place_+1) == 1) {
    attackable[2] = false;
    attackable[7] = false;
  }
  if (mp->GetMP(place_-20) == 1) {
    attackable[0] = false;
    attackable[2] = false;
  }
  if (mp->GetMP(place_+20) == 1) {
    attackable[5] = false;
    attackable[7] = false;
  }

  // 攻撃可能な位置を allObject に代入
  vector<int> allObject;
  allObject.clear();  // 初期化。
  for (int i = 0; i < 8; i++) {
    if (attackable[i] && mp->GetMP(benri(i, place_)) == 3) {
      allObject.push_back(benri(i, place_));
    }
  }

  // allObjectによって場合分け
  if (allObject.size() !=0) {  // SS1 攻撃できる場合攻撃 => 完成
    int objectPlace = allObject[mtrand()%allObject.size()];
    for (auto it_f = u_fellow->begin(); it_f != u_fellow->end(); it_f++) {
      if (it_f->GetPlace() == objectPlace) {
        // ダメージ
        it_f->Damage(at_);

        // HPによる死亡判定
        if (it_f->GetIncidentHP() < 0) {  // 死んだ場合
          *dead = *it_f;
          return;
        } else {  // 生き残った場合（分裂判定）
          if (it_f->GetSP() == 206             // 対象が仲間スモグルで、
              && mp->VacantSpace(objectPlace)  // 分裂スペースがあり、
              && mtrand()%4 == 0) {            // かつ25%を引いた場合
            // とりあえずランダムに分裂で実装
            bool flag = true;
            int random = 0;
            while (flag) {
              random = benri(mtrand()%8, objectPlace);
              if (mp->GetMP(random) == 0) {
		u_enemy->emplace_back(random);  // C++11
                // u_enemy->push_back(Enemy(random));
                mp->SetMP(random, Map::ENEMY);
                flag = false;
              }
            }
          }
        }
        break;
      }
    }
  } else {  // SS2 次に攻撃可能位置以外（要は角抜け）に敵がいるか確認 => 完成
    allObject.clear();
    // allObject に角抜け位置を代入
    for (int i = 0; i < 8; i++) {
      if (mp->GetMP(benri(i, place_)) == 3) {
        allObject.push_back(benri(i, place_));
      }
    }
    // 角抜け位置の敵の数で場合分け
    if (allObject.size() == 0) {  // SS4 角抜けにいない場合ランダムに移動 => 完成
      int num;
      if ((num = mp->Movable(place_).size()) != 0) {  // 移動スペースがある場合。無い場合は終了。
        int place = mp->Movable(place_)[mtrand()%num];
        mp->SetMP(place_, Map::EMPTY);
        place_ = place;  // Enemyインスタンスの場所を移動
        mp->SetMP(place_, Map::ENEMY);
      }
    } else if (allObject.size() == 1){  // SS3 角抜け１匹いる場合それに合わせて移動 => 完成
      int obP = allObject[0];
      switch (place_-obP) {
      case 21:
	if (mp->GetMP(place_-20) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_-20;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	} else if (mp->GetMP(place_-1) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_-1;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	}
	break;
      case 19:
	if (mp->GetMP(place_-20) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_-20;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	} else if (mp->GetMP(place_+1) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_+1;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	}
	break;
      case -19:
	if (mp->GetMP(place_+20) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_+20;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	} else if (mp->GetMP(place_-1) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_-1;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	}
	break;
      case -21:
	if (mp->GetMP(place_+20) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_+20;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	} else if (mp->GetMP(place_+1) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_+1;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	}
	break;
      }
    } else if (allObject.size() == 2) {  // SS5 角抜け２匹いる場合それに合わせて移動 => 未完成
      // TODO
    }
  }
}

void Enemy::Action(ENEMYVECTOR *u_enemy, FELLOWVECTOR *u_fellow, Map *mp) {
  // まずは攻撃可能位置に敵がいるか確認
  bool attackable[8];
  for (int i = 0; i < 8; i++) {
    attackable[i] = true;
  }
  if (mp->GetMP(place_-1) == 1) {
    attackable[0] = false;
    attackable[5] = false;
  }
  if (mp->GetMP(place_+1) == 1) {
    attackable[2] = false;
    attackable[7] = false;
  }
  if (mp->GetMP(place_-20) == 1) {
    attackable[0] = false;
    attackable[2] = false;
  }
  if (mp->GetMP(place_+20) == 1) {
    attackable[5] = false;
    attackable[7] = false;
  }

  vector<int> allObject;  // 攻撃可能な位置を格納するベクトル
  allObject.clear();  // 初期化。
  for (int i = 0; i < 8; i++) {
    if (attackable[i] && mp->GetMP(benri(i, place_)) == 3) {
      allObject.push_back(benri(i, place_));
    }
  }
  if (allObject.size() != 0) {  // SS1 攻撃できる場合攻撃 => 完成
    int objectPlace = allObject[mtrand()%allObject.size()];
    for (auto it_f = u_fellow->begin(); it_f != u_fellow->end(); it_f++) {
      if (it_f->GetPlace() == objectPlace) {
        // ダメージ
        it_f->Damage(at_);
        // std::cout << "attack!" << std::endl;
        // HPによる死亡判定
        if (it_f->GetIncidentHP() < 0) {  // 死んだ場合
          std::cout << "以下のモンスターが死にましたのでシミュレーションを終了します\n";
          it_f->ShowAll();
          exit(1);
          break;
        } else {  // 生き残った場合（分裂判定）
          // std::cout << "survived!" << std::endl;
          if (it_f->GetSP() == 206  // 対象が仲間スモグルで、
              && mp->VacantSpace(objectPlace)  // 分裂スペースがあり、
              && mtrand()%4 == 0) {  // かつ25%を引いた場合
            // とりあえずランダムに分裂で実装
            bool flag = true;
            int random = 0;
            while (flag) {
              random = benri(mtrand()%8, objectPlace);
              if (mp->GetMP(random) == 0) {
                // C++11
                u_enemy->emplace_back(random);
                // u_enemy->push_back(Enemy(random));
                mp->SetMP(random, Map::ENEMY);
                flag = false;
              }
            }
          }
        }
        break;
      }
    }
    // std::cout << "action finished!" << std::endl;
  } else {  // SS2 次に攻撃可能位置以外（要は角抜け）に敵がいるか確認 => 完成
    allObject.clear();
    for (int i = 0; i < 8; i++) {
      if (mp->GetMP(benri(i, place_)) == 3) {
        allObject.push_back(benri(i, place_));
      }
    }
    if (allObject.size() == 0) {  // SS4 角抜けにいない場合ランダムに移動 => 完成
      int num;
      if ((num=mp->Movable(place_).size()) != 0) {  // 移動スペースがある場合。無い場合は終了。
        int place = mp->Movable(place_)[mtrand()%num];
        mp->SetMP(place_, Map::EMPTY);
        place_ = place;  // Enemyインスタンスの場所を移動
        mp->SetMP(place_, Map::ENEMY);
      }
    } else if (allObject.size() == 1) {  // SS3 角抜け１匹いる場合それに合わせて移動 => 完成
      int obP = allObject[0];
      switch (place_-obP) {
      case 21:
	if (mp->GetMP(place_-20) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_-20;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	} else if (mp->GetMP(place_-1) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_-1;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	}
	break;
      case 19:
	if (mp->GetMP(place_-20) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_-20;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	} else if (mp->GetMP(place_+1) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_+1;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	}
	break;
      case -19:
	if (mp->GetMP(place_+20) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_+20;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	} else if (mp->GetMP(place_-1) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_-1;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	}
	break;
      case -21:
	if (mp->GetMP(place_+20) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_+20;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	} else if (mp->GetMP(place_+1) == 0) {
	  mp->SetMP(place_, Map::EMPTY);
	  place_ = place_+1;  // Enemyインスタンスの場所を移動
	  mp->SetMP(place_, Map::ENEMY);
	}
	break;
      }
    } else if (allObject.size() == 2) {  // SS5 角抜け２匹いる場合それに合わせて移動 => 未完成
    }
  }
}


void Enemy::ShowEXP(int n) {
  std::cout << "N=" << n << "\t" << Enemy::deadSG_*22 << std::endl;
}

void Fellow::DokuNoIki(ENEMYVECTOR *u_enemy, Map *mp) {
  bool attackable[8];       // 壁の位置によって攻撃できる対象を制限する
  for (int i = 0; i < 8; i++) {
    attackable[i] = true;
  }
  if (mp->GetMP(place_-1) == 1) {
    attackable[0] = false;
    attackable[5] = false;
  }
  if (mp->GetMP(place_+1) == 1) {
    attackable[2] = false;
    attackable[7] = false;
  }
  if (mp->GetMP(place_-20) == 1) {
    attackable[0] = false;
    attackable[2] = false;
  }
  if (mp->GetMP(place_+20) == 1) {
    attackable[5] = false;
    attackable[7] = false;
  }

  vector<int> allObject;  // 特技使用可能な位置を格納するベクトル
  int objectPlace;

  allObject.clear();  // 初期化。倍速のため。
  for (int i = 0; i < 8; i++) {
    if (attackable[i] && mp->GetMP(benri(i, place_)) == 2) {
      allObject.push_back(benri(i, place_));
    }
  }
  // 本当の対象決定
  if (allObject.size() != 0) {  // 攻撃可能な位置があったらランダムに決定
    objectPlace = allObject[mtrand()%(allObject.size())];
    for (auto it = u_enemy->begin(); it != u_enemy->end(); it++) {
      if (it->GetPlace() == objectPlace) {
        it->GetDoku();
        break;
      }
    }
  }
}

void Fellow::Kuchifuji(ENEMYVECTOR *u_enemy, Map *mp) {
  bool attackable[8];       // 壁の位置によって攻撃できる対象を制限する
  for (int i = 0; i < 8; i++) {
    attackable[i] = true;
  }
  if (mp->GetMP(place_-1) == 1) {
    attackable[0] = false;
    attackable[5] = false;
  }
  if (mp->GetMP(place_+1) == 1) {
    attackable[2] = false;
    attackable[7] = false;
  }
  if (mp->GetMP(place_-20) == 1) {
    attackable[0] = false;
    attackable[2] = false;
  }
  if (mp->GetMP(place_+20) == 1) {
    attackable[5] = false;
    attackable[7] = false;
  }

  vector<int> allObject;  // 特技使用可能な位置を格納するベクトル
  int objectPlace;

  allObject.clear();  // 初期化。
  for (int i = 0; i < 8; i++) {
    if (attackable[i] && mp->GetMP(benri(i, place_)) == 2) {  // mp==2 => enemy
      allObject.push_back(benri(i, place_));
    }
  }
  // 実際の対象決定
  if (allObject.size() != 0) {  // 攻撃可能な位置があったらランダムに決定
    objectPlace = allObject[mtrand()%(allObject.size())];
    for (auto it = u_enemy->begin(); it != u_enemy->end(); it++) {
      if (it->GetPlace() == objectPlace) {
        it->Seal();  // 封印
        break;
      }
    }
  }
}

void Enemy::GetDoku() {
  atW_ += 1;
  if (atW_ < 9) {
    at_ = GetAT(sp_, 1) * pow(0.5, atW_);
  } else if (atW_ == 9) {
    at_ = 0;
  } else {
    atW_ = 9;
    at_ = 0;
  }
}

void Fellow::Hoimi(ENEMYVECTOR *u_enemy, FELLOWVECTOR *u_fellow, Map *mp) {
  bool flag = true;  // 行動したら false になる
  int count = 0;
  // 周囲のHPが減っているユニットの数 = count
  for (int i = 0; i < 8; i++) {
    if (mp->GetMP(benri(i, place_)) == 2 || mp->GetMP(benri(i, place_)) == 3) {
      for (auto it = u_enemy->begin(); it != u_enemy->end(); it++) {
        if (it->GetPlace() == benri(i, place_)
            && it->GetIncidentHP() < it->GetMHP()) {
          count++;
          break;
        }
      }
      for (auto it_f = u_fellow->begin(); it_f != u_fellow->end(); it_f++) {
        if (it_f->GetPlace() == benri(i, place_)
            && it_f->GetIncidentHP() < it_f->GetMHP()) {
          count++;
          break;
        }
      }
    }
  }
  for (int i = 0; i < count; i++) {
    if (mtrand()%10 < 3) {  // ホイミ率は30%
      flag = false;
      for (auto it = u_enemy->begin(); it != u_enemy->end(); it++) {
        for (int j = 0; j < 8; j++) {
          if (it->GetPlace() == benri(j, place_)) it->Get25();
        }
      }
      for (auto it_f = u_fellow->begin(); it_f != u_fellow->end(); it_f++) {
        for (int j  = 0; j < 8; j++) {
          if (it_f->GetPlace() == benri(j, place_)) {
            it_f->Get25();
          }
        }
      }
      break;
    }
  }

  if (flag && mtrand()%3 == 0) {  // 現状では33%の確率で殴る事にしておく。
    this->Attack(u_enemy, mp);
  }
}


void Fellow::Attack(ENEMYVECTOR *u_enemy, Map *mp) {
  bool attackable[8];
  for (int i = 0; i < 8; i++) {
    attackable[i] = true;
  }
  if (mp->GetMP(place_-1) == 1) {
    attackable[0] = false;
    attackable[5] = false;
  }
  if (mp->GetMP(place_+1) == 1) {
    attackable[2] = false;
    attackable[7] = false;
  }
  if (mp->GetMP(place_-20) == 1) {
    attackable[0] = false;
    attackable[2] = false;
  }
  if (mp->GetMP(place_+20) == 1) {
    attackable[5] = false;
    attackable[7] = false;
  }

  vector<int> allObject;  // 攻撃可能な位置を格納するベクトル
  int objectPlace;

  allObject.clear();  // 初期化。倍速のため。
  for (int i = 0; i < 8; i++) {
    if (attackable[i] && mp->GetMP(benri(i, place_)) == 2) {
      allObject.push_back(benri(i, place_));
    }
  }
  // 本当の対象決定
  if (allObject.size() != 0) {  // 攻撃可能な位置があったらランダムに決定
    objectPlace = allObject[mtrand()%(allObject.size())];
    // vector iterator
    for (auto it = u_enemy->begin(); it != u_enemy->end(); it++) {
      if (it->GetPlace() == objectPlace) {
        int howmany = 1;
        if (sp_ == 613 && !seal_) howmany = 2;  // キラーマ用
        if (sp_ == 314 && !seal_) howmany = 2;  // かまきち用
        if (sp_ == 655 && !seal_) howmany = 3;  // キラーマ用
        for (int i = 0; i < howmany; i++) {
          // ダメージ
          it->Damage(at_);
          // HPによる死亡判定
          if (it->GetIncidentHP() < 0) {  // 死んだ場合
            it->Incliment();
            mp->SetMP(objectPlace, Map::EMPTY);
            u_enemy->erase(it);
            exp_ += 22;
            totalEXP_ += 22;
            LVupCheck();
            break;
          } else {  // 生き残った場合（分裂判定）
            if (mp->VacantSpace(objectPlace)  // 分裂スペースがあり、
                && mtrand()%4 == 0) {  // かつ25%を引いた場合
              // 完全には実装されていない関数
              Direction(this->place_, objectPlace, mp, u_enemy);
            }
          }
        }
        break;
      }
    }
  }
}

void Fellow::Fire(ENEMYVECTOR *u_enemy, Map *mp, int damage) {
  vector<int> allObject;  // 攻撃可能な位置を格納するベクトル
  int objectPlace;

  allObject.clear();  // 初期化。倍速のため。
  for (int i = 0; i < 8; i++) {
    if (mp->GetMP(benri(i, place_)) == 2) {
      allObject.push_back(benri(i, place_));
    }
  }
  // 本当の対象決定
  if (allObject.size() != 0) {  // 攻撃可能な位置があったらランダムに決定
    objectPlace = allObject[mtrand()%(allObject.size())];
    // vector iterator
    for (auto it = u_enemy->begin(); it != u_enemy->end(); it++) {
      if (it->GetPlace() == objectPlace) {
        // ダメージ
        it->Wounded(damage);
        // HPによる死亡判定
        if (it->GetIncidentHP() < 0) {  // 死んだ場合
          it->Incliment();
          mp->SetMP(objectPlace, Map::EMPTY);
          u_enemy->erase(it);
          exp_ += 22;
          totalEXP_ += 22;
          LVupCheck();
          break;
        } else {  // 生き残った場合（分裂判定）
          if (mp->VacantSpace(objectPlace)  // 分裂スペースがあり、
              && mtrand()%4 == 0) {  // かつ25%を引いた場合
            // 完全には実装されていない関数
            Direction(this->place_, objectPlace, mp, u_enemy);
          }
        }
        break;
      }
    }
  }
}


void Fellow::MoonSault(ENEMYVECTOR *u_enemy, Map *mp) {
  bool attackable[8];
  for (int i = 0; i < 8; i++) {
    attackable[i] = true;
  }
  if (mp->GetMP(place_-1) == 1) {
    attackable[0] = false;
    attackable[5] = false;
  }
  if (mp->GetMP(place_+1) == 1) {
    attackable[2] = false;
    attackable[7] = false;
  }
  if (mp->GetMP(place_-20) == 1) {
    attackable[0] = false;
    attackable[2] = false;
  }
  if (mp->GetMP(place_+20) == 1) {
    attackable[5] = false;
    attackable[7] = false;
  }

  vector<int> allObject;  // 攻撃可能な位置を格納するベクトル
  int objectPlace;

  allObject.clear();  // 初期化。倍速のため。
  for (int i = 0; i < 8; i++) {
    if (attackable[i] && mp->GetMP(benri(i, place_)) == 2) {
      allObject.push_back(benri(i, place_));
    }
  }
  // 本当の対象決定
  if (allObject.size() != 0) {  // 攻撃可能な位置があったらランダムに決定
    objectPlace = allObject[mtrand()%(allObject.size())];
    // vector iterator
    for (auto it = u_enemy->begin(); it != u_enemy->end(); it++) {
      if (it->GetPlace() == objectPlace) {
        // ダメージ
        it->Lose25();
        // HPによる死亡判定
        if (it->GetIncidentHP() < 0) {  // 死んだ場合
          it->Incliment();
          mp->SetMP(objectPlace, Map::EMPTY);
          u_enemy->erase(it);
          exp_ += 22;
          totalEXP_ += 22;
          LVupCheck();
          break;
        } else {  // 生き残った場合（分裂判定）
          if (mp->VacantSpace(objectPlace)  // 分裂スペースがあり、
              && mtrand()%4 == 0) {  // かつ25%を引いた場合
            // 完全には実装されていない関数
            Direction(this->place_, objectPlace, mp, u_enemy);
          }
        }
        break;
      }
    }
  }
}

void Fellow::LVupCheck() {
  if (totalEXP_ >= GetEXP(sp_, lv_+1)) {
    lv_ += 1;
    mhp_ = GetHP(sp_, lv_) + dp_;
    if (atW_ == 0) {
      at_ = GetAT(sp_, lv_);
    } else if (atW_ < 9) {
      at_ = GetAT(sp_, lv_) * pow(0.5, atW_) + 0.5;
    } else if (atW_ == 9) {
      at_ = 0;
    } else {
    }

    switch (dfW_) {
    case 0:
      df_ = GetDF(sp_, lv_);
      break;
    case 1:
      df_ = GetDF(sp_, lv_) * 0.8 + 0.5;
      break;
    case 2:
      df_ = GetDF(sp_, lv_) * 0.7 + 0.5;
      break;
    case 3:
      df_ = GetDF(sp_, lv_) * 0.5 + 0.5;
      break;
    case 4:
      df_ = GetDF(sp_, lv_) * 0.4 + 0.5;
      break;
    case 5:
      df_ = GetDF(sp_, lv_) * 0.2 + 0.5;
      break;
    case 6:
      df_ = GetDF(sp_, lv_) * 0.001 + 0.5;
      break;
    default:
      printf("RUKANI > 7 is not good!\n");
      exit(2);
    }
  }
}

void Unit::Damage(double at) {
  if (mtrand()%100 < 92) {
    int dm = static_cast<int>(floor(at*1.3*pow(35, df_)/pow(36, df_)
                                    *(112+mtrand()%32)/128 + 0.5));
    if (dm != 0) {
      hp_ -= dm;
    } else {
      hp_ -= 1;
    }
  }
}

/*! @brief 絶対位置 i に対して周囲の相対位置 j の絶対位置を返す関数
  @param[in] i 絶対位置
  @param[in] j 相対位置
*/
int benri(int i, int j) {
  switch (i) {
  case 0:
    return j-21;
    break;
  case 1:
    return j-20;
    break;
  case 2:
    return j-19;
    break;
  case 3:
    return j-1;
    break;
  case 4:
    return j+1;
    break;
  case 5:
    return j+19;
    break;
  case 6:
    return j+20;
    break;
  case 7:
    return j+21;
    break;
  default:
    std::cout << "fucking benri(" << i << ", " << j << ")\n";
    exit(1);
  }
  return 0;
}

void Fellow::ShowAll() {
  std::cout << "#########################" << std::endl
            << "ID:\t" << id_ << std::endl
            << "Name:\t" << GetName(sp_) << std::endl
            << "LV:\t" << lv_ << std::endl
            << "MHP:\t" << mhp_ << std::endl
            << "AT:\t" << at_ << std::endl
            << "DF:\t" << df_ << std::endl
            << "speed\t" << speed_ << std::endl
            << "exp\t" << exp_ << std::endl
            << "totexp\t" << totalEXP_ << std::endl
            << "x axis:\t" << 1 + place_%20 << std::endl
            << "y axis:\t" << 1 + place_/20 << std::endl
            <<  "#########################" << std::endl;
}

void Unit::ShowData() {
  std::cout << "#########################" << std::endl
            << "Name:\t" << GetName(sp_) << std::endl
            << "MHP:\t" << mhp_ << std::endl
            << "AT:\t" << at_ << std::endl
            << "DF:\t" << df_ << std::endl
            << "x axis:\t" << 1 + place_%20 << std::endl
            << "y axis:\t" << 1 + place_/20 << std::endl
            <<  "#########################" << std::endl;
}

void Unit::Seal() {
  seal_ = true;
}

Fellow::Fellow(int place, int sp, int lv, int dp, int speed,
               int atWeakenTimes, int dfWeakenTimes, bool seal) {
  id_ = -1;
  seal_ = seal;
  place_ = place;
  sp_ = sp;
  lv_ = lv;
  dp_ = dp;
  speed_ = speed;
  exp_ = 0;
  totalEXP_ = GetEXP(sp, lv);
  atW_ = atWeakenTimes;
  dfW_ = dfWeakenTimes;
  seal_ = seal;

  mhp_ = GetHP(sp, lv) + dp;
  rec_ = GetREC(sp);
  hp_ = mhp_;
  if (atWeakenTimes == 0) {
    at_ = GetAT(sp, lv);
  } else if (atWeakenTimes < 9) {
    at_ = GetAT(sp, lv) * pow(0.5, atWeakenTimes);
  } else if (atWeakenTimes == 9) {
    at_ = 0;
  } else {
  }

  switch (dfWeakenTimes) {
  case 0:
    df_ = GetDF(sp, lv);
    break;
  case 1:
    df_ = GetDF(sp, lv) * 0.8;
    break;
  case 2:
    df_ = GetDF(sp, lv) * 0.7;
    break;
  case 3:
    df_ = GetDF(sp, lv) * 0.5;
    break;
  case 4:
    df_ = GetDF(sp, lv) * 0.4;
    break;
  case 5:
    df_ = GetDF(sp, lv) * 0.2;
    break;
  case 6:
    df_ = GetDF(sp, lv) * 0.001;
    break;
  default:
    printf("RUKANI > 7 is not good !\n");
    exit(2);
  }
}

Enemy::Enemy(int place) {
  sp_ = 206;
  mhp_ = 55;
  at_ = 17;
  df_ = 10;
  hp_ = 55;
  atW_ = 0;               // キノコ用に2014.5.23追加
  place_ = place;
  seal_ = false;
}

void Direction(const int thisPlace, const int objectPlace,
               Map* mp, ENEMYVECTOR *u_enemy) {
  Enemy::directionAll_++;
  /* 
   * 関数の概要:
   * (1) thisPlace   -> 攻撃したモンスターの位置
   *     objectPlace -> 攻撃対象モンスターの位置
   * (2) プロセス
   *    + マップと位置を読み込み、変換する
   *    + 分裂箇所を決める
   *    + 逆変換し、分裂先の位置を返す
   */


  /*
   *  マップ情報を読み込み、変換してtmpに代入
   *（十字の場合は下、斜めのときは右下を基準にする）
   */
  int tmp[8];  // 周囲の情報
  int tmpThis = 0;  // 攻撃したモンスターの変換後の位置
  bool placeFlag = true;
  switch (objectPlace - thisPlace) {
  case -1:  // 右
    tmpThis = thisPlace + 19;
  case -19:  // 左下
    if (tmpThis == 0) {
      tmpThis = thisPlace + 2;
      placeFlag = false;
    }
    tmp[0] = mp->GetMP(benri(2, objectPlace));
    tmp[1] = mp->GetMP(benri(4, objectPlace));
    tmp[2] = mp->GetMP(benri(7, objectPlace));
    tmp[3] = mp->GetMP(benri(1, objectPlace));
    tmp[4] = mp->GetMP(benri(6, objectPlace));
    tmp[5] = mp->GetMP(benri(0, objectPlace));
    tmp[6] = mp->GetMP(benri(3, objectPlace));
    tmp[7] = mp->GetMP(benri(5, objectPlace));
    break;
  case 1:  // 左
    tmpThis = thisPlace + 21;
  case 19:  // 右上
    if (tmpThis == 0) {
      placeFlag = false;
      tmpThis = thisPlace + 40;
    }
    tmp[0] = mp->GetMP(benri(5, objectPlace));
    tmp[1] = mp->GetMP(benri(3, objectPlace));
    tmp[2] = mp->GetMP(benri(0, objectPlace));
    tmp[3] = mp->GetMP(benri(6, objectPlace));
    tmp[4] = mp->GetMP(benri(1, objectPlace));
    tmp[5] = mp->GetMP(benri(7, objectPlace));
    tmp[6] = mp->GetMP(benri(4, objectPlace));
    tmp[7] = mp->GetMP(benri(2, objectPlace));
    break;
  case -20:  // 下(base)
  case -21:  // 右下(base)
    if (objectPlace - thisPlace == -21) placeFlag = false;
    tmpThis = thisPlace;
    for (int i = 0; i < 8; i++) {
      tmp[i] = mp->GetMP(benri(i, objectPlace));
    }
    break;
  case 20:  // 上
    tmpThis = thisPlace + 40;
  case 21:  // 左上
    if (tmpThis == 0) {
      placeFlag = false;
      tmpThis = thisPlace + 42;
    }
    tmp[0] = mp->GetMP(benri(7, objectPlace));
    tmp[1] = mp->GetMP(benri(6, objectPlace));
    tmp[2] = mp->GetMP(benri(5, objectPlace));
    tmp[3] = mp->GetMP(benri(4, objectPlace));
    tmp[4] = mp->GetMP(benri(3, objectPlace));
    tmp[5] = mp->GetMP(benri(2, objectPlace));
    tmp[6] = mp->GetMP(benri(1, objectPlace));
    tmp[7] = mp->GetMP(benri(0, objectPlace));
    break;
  default:
    std::cout << "Problem occured at division direction.\n";
    exit(1);
  }


  // 主要部

  /*
   * 周囲の空白が1マスしか無い場合は最後の else にまわしてもいいけど、
   * else に到達するまでに時間がかかりそう。
   * 逆に1マスの場合を else if に入れたら、その条件判定の時間がかかりそう。
   * どっちがいいかわからないけど、前者の場合書く量が減るから前者を取る。
   * 2013/3/19
   */
  int future = 0;  // 0-7を収納後、最終は位置を収納。


  if ( tmp[0] == 0 &&  // tmp[]==0 EMPTY
       tmp[1] == 0 &&  // tmp[]!=0 WALL, ENEMY, SUMMON
       tmp[2] == 0 &&
       tmp[3] != 0 &&
       tmp[4] != 0 &&
       tmp[5] != 0 &&
       tmp[6] != 0 &&
       tmp[7] != 0
       ) {
    /*
     * ---
     * +O+
     * +A+
     */
    if (mtrand()%4 == 0) {
      future = 1;
    } else {
      future = 2;
    }
  } else if (
	     tmp[0] == 0 &&
	     tmp[1] == 0 &&  // tmp[]==0 EMPTY
	     tmp[2] != 0 &&  // tmp[]!=0 WALL, ENEMY, SUMMON
	     tmp[3] != 0 &&
	     tmp[4] != 0 &&
	     tmp[5] != 0 &&
	     tmp[6] != 0 &&
	     tmp[7] != 0
             ) {
    /*
     * --+
     * +O+
     * +A+
     */
    future = 1;
  } else if (
	     tmp[0] != 0 &&
	     tmp[1] == 0 &&  // tmp[]==0 EMPTY
	     tmp[2] == 0 &&  // tmp[]!=0 WALL, ENEMY, SUMMON
	     tmp[3] != 0 &&
	     tmp[4] != 0 &&
	     tmp[5] != 0 &&
	     tmp[6] != 0 &&
	     tmp[7] != 0
             ) {
    /*
     * +--
     * +O+
     * +A+
     */
    if (mtrand()%4 == 0) {
      future = 1;
    } else {
      future = 2;
    }
  } else if (
	     tmp[0] == 0 &&
	     tmp[1] != 0 &&  // tmp[]==0 EMPTY
	     tmp[2] == 0 &&  // tmp[]!=0 WALL, ENEMY, SUMMON
	     tmp[3] != 0 &&
	     tmp[4] != 0 &&
	     tmp[5] != 0 &&
	     tmp[6] != 0 &&
	     tmp[7] != 0
             ) {
    /*
     * -+-
     * +O+
     * +A+
     */
    if (mtrand()%4 == 0) {
      future = 0;
    } else {
      future = 2;
    }
  } else {  // 未調査の場合。とりあえずランダムで返す
    bool flag = true;
    while (flag) {
      future = mtrand()%8;
      if (tmp[future] == Map::EMPTY) {
        flag = false;
      }
    }


    int check = 0;
    for (int i = 0; i < 8; i++) {
      if (tmp[i] == 0) {
        check++;
      }
    }
    if (check != 1) {
      Enemy::directionElse_++;
    }
  }


  // std::cout << "before 逆変換: " <<  future << std::endl;

  // 逆変換 => 完了
  switch (objectPlace - thisPlace) {
  case -1:  // 右
  case -19:  // 左下
    switch (future) {
    case 0:
      future = benri(2, objectPlace);
      break;
    case 1:
      future = benri(4, objectPlace);
      break;
    case 2:
      future = benri(7, objectPlace);
      break;
    case 3:
      future = benri(1, objectPlace);
      break;
    case 4:
      future = benri(6, objectPlace);
      break;
    case 5:
      future = benri(0, objectPlace);
      break;
    case 6:
      future = benri(3, objectPlace);
      break;
    case 7:
      future = benri(5, objectPlace);
      break;
    default:
      std::cout << "Problem occured at ANIT CONVERSION" << std::endl;
      exit(1);
    }
    break;
  case 1:  // 左
  case 19:  // 右上
    switch (future) {
    case 0:
      future = benri(5, objectPlace);
      break;
    case 1:
      future = benri(3, objectPlace);
      break;
    case 2:
      future = benri(0, objectPlace);
      break;
    case 3:
      future = benri(6, objectPlace);
      break;
    case 4:
      future = benri(1, objectPlace);
      break;
    case 5:
      future = benri(7, objectPlace);
      break;
    case 6:
      future = benri(4, objectPlace);
      break;
    case 7:
      future = benri(2, objectPlace);
      break;
    default:
      std::cout << "Problem occured at ANIT CONVERSION" << std::endl;
      exit(1);
    }
    break;
  case -20:  // 下(base)
  case -21:  // 右下(base)
    future = benri(future, objectPlace);
    break;
  case 20:  // 上
  case 21:  // 左上
    switch (future) {
    case 0:
      future = benri(7, objectPlace);
      break;
    case 1:
      future = benri(6, objectPlace);
      break;
    case 2:
      future = benri(5, objectPlace);
      break;
    case 3:
      future = benri(4, objectPlace);
      break;
    case 4:
      future = benri(3, objectPlace);
      break;
    case 5:
      future = benri(2, objectPlace);
      break;
    case 6:
      future = benri(1, objectPlace);
      break;
    case 7:
      future = benri(0, objectPlace);
      break;
    default:
      std::cout << "Problem occured at ANIT CONVERSION" << std::endl;
      exit(1);
    }
    break;
  default:
    std::cout << "Problem occured at ANIT CONVERSION" << std::endl;
    exit(1);
  }


  if (mp->GetMP(future) != Map::EMPTY) {
    mp -> ShowMap10x10();
    std::cout << future << std::endl;
    std::cout << "不正なマスに分裂しました\n";
  }
  // C++11
  u_enemy->emplace_back(future);
  // u_enemy->push_back(Enemy(future));
  mp->SetMP(future, Map::ENEMY);
}
