#pragma once
/*
 * 敵スモグル（Enemy）と仲間（Fellow）のクラス
 *
 */

#include <vector>
#include <cstdlib>

class Map;
class Unit;
class Fellow;
class Enemy;
typedef std::vector<Fellow> FELLOWVECTOR;
typedef std::vector<Enemy> ENEMYVECTOR;
typedef std::vector<Unit> UNITVECTOR;

class Unit {
 public:
  void Seal();
  void ShowData();
  int GetPlace() { return place_; }
  int GetIncidentHP() { return hp_; }
  int GetMHP() { return mhp_; }
  int GetSP() { return sp_; }
  void Get25() {
    hp_+=25;
    if (hp_>mhp_) {
      hp_ = mhp_;
    }
  };
  void Lose25();
  void Wounded(int damage) {
      hp_-=damage;
  }
  void Damage(double at);

 protected:
  int sp_;
  int mhp_;
  double at_;
  double df_;
  bool seal_;
  double hp_;
  int place_;
};

class Fellow : public Unit {
 public:
  Fellow();
  Fellow(int place, int sp, int lv,
         int dp, int speed,
         int atWeakenTimes, int dfWeakenTimes, bool seal);
  Fellow& operator=(const Fellow& fl);
  void Action(ENEMYVECTOR *u_enemy, FELLOWVECTOR *u_fellow, Map *mp);
  void Attack(ENEMYVECTOR *u_enemy, Map *mp);
  // ホイミンの行動プロセス全て
  void Hoimi(ENEMYVECTOR *u_enemy, FELLOWVECTOR *u_fellow, Map *mp);
  // キノコ用に2014.5.23追加
  void DokuNoIki(ENEMYVECTOR *u_enemy, Map *mp);
  // ムーンサルト2014.5.23追加
  void MoonSault(ENEMYVECTOR *u_enemy, Map *mp);
  // 炎の息2014.5.25追加
  void Fire(ENEMYVECTOR *u_enemy, Map *mp, int damage);
  // 口封じ2014.7.25追加
  void Kuchifuji(ENEMYVECTOR *u_enemy, Map *mp);
  void LVupCheck();
  void ShowData();
  void ShowAll();
  void SetID(int id) { id_ = id; }
  int GetID() { return id_; }
  int GetSPEED() { return speed_; }
  int GetEXP_() { return exp_; }

 private:
  int id_;
  int lv_;
  int dp_;
  int atW_;
  int dfW_;
  int speed_;
  double rec_;
  unsigned int exp_;
  unsigned int totalEXP_;
};

class Enemy : public Unit {
 public:
  explicit Enemy(int place);
  void GetDoku();
  void Incliment() { deadSG_++; }
  void ClearDeadSG() { deadSG_ = 0; }
  static int GetDeadSG() { return deadSG_*22; }
  static void ShowEXP(int n);
  void Action(ENEMYVECTOR *u_enemy, FELLOWVECTOR *u_fellow, Map *mp, Fellow *dead);
  void Action(ENEMYVECTOR *u_enemy, FELLOWVECTOR *u_fellow, Map *mp);
  static int directionElse_;
  static int directionAll_;
 private:
  int atW_;  // キノコ用に2014.5.23追加
  static int deadSG_;
};

int benri(int i, int j);
void Direction(const int thisPlace, const int objectPlace, Map* mp, ENEMYVECTOR *u_enemy);
