#pragma once
#include "unit.h"
#include "map.h"
/*
 * インプットファイルを扱うクラス
 *  - MakeUnits : ユニットのデータを入れる
 *  - MakeMap   : マップのデータを入れる
 */
class Input {
 public:
  Input(std::string filename);
  int GetNumberOfSummon() { return numberOfSummon; }
  int GetSP(int i) { return sp_[i]; }
  int GetLV(int i) { return lv_[i]; }
  int GetOrder(int i) { return order_[i]; }
  void MakeUnits(FELLOWVECTOR *u_fellow, ENEMYVECTOR *u_enemy);
  void MakeMap(Map *mp);
  void SetSpeed(int i);
  void SetMode(int i);
  void DeleteComment(std::string &buf);
  void DeleteHeadSpace(std::string &buf);
  void DeleteSpace(std::string &buf);
  enum Mode {
    EFFICIENCY,
    DURABILITY,
    SAFETY,
    DEBUG,
  };
  // fundamental
  unsigned int N_;
  unsigned int T_;
  Mode MODE_;
  
 private:
  // map
  int mp_[400];
  // monster
  int seal_[10];
  int place_[10];
  int sp_[10];
  int lv_[10];
  int dp_[10];
  int speed_[10];
  int atW_[10];
  int dfW_[10];
  // order
  int order_[10];
  int numberOfSummon;
  int numberOfEnemy;
};
