// Copyright ikarino99 2014
#include <regex>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include "input.h"
#include "unit.h"
#include "map.h"

using std::vector;
using std::string;
using std::regex;
void Input::SetMode(int i) {
  switch (i) {
  case -1:
    break;
  case 0:
    std::cout << "モード選択オプションで効率計算モードに設定されました\n";
    MODE_ = EFFICIENCY;
    break;
  case 1:
    std::cout << "モード選択オプションで持続性計算モードに設定されました\n";
    MODE_ = DURABILITY;
    break;
  case 2:
    std::cout << "モード選択オプションで安定性計算モードに設定されました\n";
    MODE_ = SAFETY;
    break;
  case 3:
    std::cout << "モード選択オプションでデバッグモードに設定されました\n";
    MODE_ = DEBUG;
    break;
  default:
    std::cout << "モード選択オプションで指定された値が不正です\n";
    exit(1);
  }
}

void Input::SetSpeed(int i) {
  if (i == 1||i == 2) {    // 同時に設定
    for (int j = 0; j < 10; j++) {
      speed_[j] = i;
    }
    std::cout << "速度オプションで、すべてのモンスターが";
    if (i == 1) {
      std::cout << "等速に設定されました\n";
    } else {
      std::cout << "倍速に設定されました\n";
    }
  } else if (i == 0) {    // インプットファイルと同じ
  } else {    // 無効な値
    std::cout << "オプションで設定された速度が異常です\n";
    exit(1);
  }
}

void Input::MakeMap(Map *mp) {
  for (int i = 0; i < 400; i++) {
    switch (mp_[i]) {
    case 0:
      mp->SetMP(i, Map::EMPTY);
      break;
    case 1:
      mp->SetMP(i, Map::WALL);
      break;
    case 2:
      mp->SetMP(i, Map::ENEMY);
      break;
    case 3:
      mp->SetMP(i, Map::SUMMON);
      break;
    }
  }
}

void Input::MakeUnits(FELLOWVECTOR *u_fellow, ENEMYVECTOR *u_enemy) {
  int i;
  for (int j = 0; j < 10; j++) {
    i = order_[j];
    if (place_[i] != -1) {
      u_fellow->push_back(Fellow(place_[i], sp_[i], lv_[i], dp_[i], speed_[i], atW_[i], dfW_[i], seal_[i]));
      // 最後のやつにID付与
      u_fellow->back().SetID(i);
    }
  }
  for (int i = 0; i < 400; i++) {
    if (mp_[i] == 2) {
      u_enemy->push_back(Enemy(i));
    }
  }
}

Input::Input(string filename) {
  // 初期化とデフォルトパラメータ代入
  numberOfSummon = 0;
  for (int i = 0; i < 10; i++) {
    place_[i] = -1;
    mp_[i] = -1;
    sp_[i] = -1;
    lv_[i] = -1;
    dp_[i] = 0;
    speed_[i] = 2;
    atW_[i] = 0;
    dfW_[i] = 0;
    order_[i] = -1;
    seal_[i] = false;
  }
  T_ = 0;
  N_ = 0;
  // tag flag
  bool fundFlag = false;
  bool mapFlag = false;
  bool monsterFlag = false;
  bool orderFlag = false;
  int id_now = 0;
  int j = 0;
  bool enemy = false;
  // インプットファイルチェック用
  vector<int> map_vec;
  vector<int> order_vec;
  vector<int> monster_vec;
  int order_id = 0;
  // regex
  std::smatch m;
  // file
  std::ifstream ifs(filename);
  if (ifs.fail()) {
    std::cout << "エラー:インプットファイルが見つかりません\n";
    exit(1);
  }
  string str;

  // インプットファイル名
  std::cout << "\n\nscs :: インプットファイル[ "
            << filename
            << " ]を読み込んでいます\n";
  // 一行ずつファイル読み込み
  while (getline(ifs, str)) {
    // 空白削除
    DeleteHeadSpace(str);
    // コメント行読み飛ばし
    char fc = str[0];
    char comment[] = "#";
    if (str == "" || fc == comment[0]) { continue; }
    DeleteSpace(str);
    // フラグ
    if (str == "<FUNDAMENTAL>") {
      fundFlag = true;
      continue;
    }
    if (str == "<MAP>") {
      mapFlag = true;
      continue;
    }
    if (str == "<MONSTER>") {
      monsterFlag = true;
      continue;
    }
    if (str == "<ORDER>") {
      orderFlag = true;
      continue;
    }
    if (str == "</FUNDAMENTAL>") {
      fundFlag = false;
      continue;
    }
    if (str == "</MAP>") {
      mapFlag = false;
      continue;
    }
    if (str == "</MONSTER>") {
      monsterFlag = false;
      continue;
    }
    if (str == "</ORDER>") {
      orderFlag = false;
      continue;
    }

    // FUNDAMENTAL
    if (fundFlag) {
      if (regex_search(str, m, regex("N=\\d+"))) {
        int tmp = atoi((regex_replace(str, regex("N="), "")).c_str());
        if (tmp < 0) {
          std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
                    << "エラー：\n"
                    << "ERROR! 試行回数Nが不正です。\n"
                    << "正しい試行回数を入力してください。\n"
                    << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
          exit(1);
        } else {
          N_ = tmp;
        }
      } else if (regex_search(str, m, regex("T=\\d+"))) {
        int tmp = atoi((regex_replace(str, regex("T="), "")).c_str());
        if (tmp < 0 || tmp > 3000) {
          std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
                    << "エラー：\n"
                    << "ERROR! ターン数Tが不正です。\n"
                    << "0以上3000以下の数値を入力してください。\n"
                    << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
          exit(1);
        } else {
          T_ = tmp;
        }
      } else if (regex_search(str, m, regex("MODE=\\d+"))) {
        switch (atoi((regex_replace(str, regex("MODE="), "")).c_str())) {
          case 0:
            MODE_ = EFFICIENCY;
            break;
          case 1:
            MODE_ = DURABILITY;
            break;
          case 2:
            MODE_ = SAFETY;
            break;
          case 3:
            MODE_ = DEBUG;
            break;
          default:
            std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
                      << "エラー：\n"
                      << "モード選択が不正です。\n"
                      << "正しいモードを入力してください。\n"
                      << "MODE 0: 効率モード\n"
                      << "\t経験値情報を表示します。\n"
                      << "MODE 1: 持続性モード\n"
                      << "\tスモグルが尽きないか確認します。\n"
                      << "MODE 2: 安全性モード\n"
                      << "\t安全性を確認します。\n"
                      << "MODE 3: デバッグモード\n"
                      << "\tランボー怒りのバグ修正\n"
                      << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
            exit(1);
            break;
        }
      }
    }
    // MAP
    if (mapFlag) {
      for (int i = 0; i < 20; i++) {
        switch (str[i]) {
          case '+':
            mp_[i+j*20] = 1;
            break;
          case '-':
            mp_[i+j*20] = 0;
            break;
          case '*':
            mp_[i+j*20] = 2;
            numberOfEnemy++;
            break;
          case '0':
            place_[0] = i+j*20;
            mp_[i+j*20] = 3;
            map_vec.push_back(0);
            break;
          case '1':
            place_[1] = i+j*20;
            mp_[i+j*20] = 3;
            map_vec.push_back(1);
            break;
          case '2':
            place_[2] = i+j*20;
            mp_[i+j*20] = 3;
            map_vec.push_back(2);
            break;
          case '3':
            place_[3] = i+j*20;
            mp_[i+j*20] = 3;
            map_vec.push_back(3);
            break;
          case '4':
            place_[4] = i+j*20;
            mp_[i+j*20] = 3;
            map_vec.push_back(4);
            break;
          case '5':
            place_[5] = i+j*20;
            mp_[i+j*20] = 3;
            map_vec.push_back(5);
            break;
          case '6':
            place_[6] = i+j*20;
            mp_[i+j*20] = 3;
            map_vec.push_back(6);
            break;
          case '7':
            place_[7] = i+j*20;
            mp_[i+j*20] = 3;
            map_vec.push_back(7);
            break;
          case '8':
            place_[8] = i+j*20;
            mp_[i+j*20] = 3;
            map_vec.push_back(8);
            break;
          case '9':
            place_[9] = i+j*20;
            mp_[i+j*20] = 3;
            map_vec.push_back(9);
            break;
          default:
            std::cout << "エラー：マップセクションの値が以上です。\n"
                      << "読み取り中に " << j+1
                      << " 行目の値でエラーが発生しました。\n";
            exit(1);
            break;
        }
      }
      // cout << endl;
      j++;
    }
    // MONSTER
    if (monsterFlag) {
      if (regex_search(str, m, regex("\\[\\d\\]"))) {
        id_now = atoi((regex_replace(str,
                                     regex("[\\[\\]]"),
                                     ""))
                      .c_str());
        monster_vec.push_back(id_now);
      } else if (regex_search(str, m, regex("type=\\d{3}"))) {
        sp_[id_now] = atoi((regex_replace(str, regex("type="), "")).c_str());
      } else if (regex_search(str, m, regex("lv=\\d+"))) {
        lv_[id_now] = atoi((regex_replace(str, regex("lv="), "")).c_str());
        if (lv_[id_now]<1 || lv_[id_now]>99) {
          std::cout << "エラー：\n"
                    << "モンスターID :" << id_now << " のレベルが不正です\n";
          exit(1);
        }
      } else if (regex_search(str, m, regex("doping=\\d+"))) {
        dp_[id_now] = atoi((regex_replace(str, regex("doping="), "")).c_str());
        if (dp_[id_now] < 0) {
          std::cout << "エラー：\n"
                    << "モンスターID :" << id_now << " のHPドーピングが不正です\n";
          exit(1);
        }
      } else if (regex_search(str, m, regex("speed=\\d+"))) {
        speed_[id_now] = atoi((regex_replace(str, regex("speed="), "")).c_str());
        if (speed_[id_now] != 1 && speed_[id_now] != 2) {
          std::cout << "エラー：\n"
                    << "モンスターID :" << id_now << " の速度が不正です\n";
          exit(1);
        }
      } else if (regex_search(str, m, regex("mizu=\\d+"))) {
        atW_[id_now] = atoi((regex_replace(str, regex("mizu="), "")).c_str());
        if (atW_[id_now]<0 || atW_[id_now]>9) {
          std::cout << "エラー：\n"
                    << "モンスターID :" << id_now << " の攻撃力弱化回数が不正です\n";
          exit(1);
        }
      } else if (regex_search(str, m, regex("rukani=\\d+"))) {
        dfW_[id_now] = atoi((regex_replace(str, regex("rukani="), "")).c_str());
        if (dfW_[id_now]<0 || dfW_[id_now]>9) {
          std::cout << "エラー：\n"
                    << "モンスターID :" << id_now << " の攻撃力弱化回数が不正です\n";
          exit(1);
        }
      } else if (regex_search(str, m, regex("fuin=\\d"))) {
        if (atoi((regex_replace(str, regex("fuin="), "")).c_str()) == 1) {
          seal_[id_now] = true;
        }
      }
    }
    // ORDER
    if (orderFlag) {
      order_[order_id] = stoi(str);
      order_id++;
      order_vec.push_back(stoi(str));
    }
  }

  // 確認用ベクトルのソート
  sort(map_vec.begin(), map_vec.end());
  sort(monster_vec.begin(), monster_vec.end());
  sort(order_vec.begin(), order_vec.end());



  // 読み込みチェック


  if (T_ == 0) {
    std::cout << "エラー：\n"
              << "ターン数が設定されていません。基礎セクションを見直してください。\n";
    exit(1);
  }
  if (N_ == 0) {
    std::cout << "エラー：\n"
              << "試行回数が設定されていません。基礎セクションを見直してください。\n";
    exit(1);
  }

  for (int i = 0; i < 400; i++) {
    if (mp_[i] == 2) {
      enemy = true;
    }
    if (i/20 == 0 || i/20 == 19) {
      if (mp_[i] != 1) {
        std::cout << "エラー：\n"
                  << "一番外側のマスが壁になっていません。マップセクションを見直してください。\n";
        exit(1);
      }
    } else if (i%20 == 0 || i%20 == 19) {
      if (mp_[i] != 1) {
        std::cout << "エラー：\n"
                  << "一番外側のマスが壁になっていません。マップセクションを見直してください。\n";
        exit(1);
      }
    }
  }
  if (!enemy) {
    std::cout << "エラー：\n"
              << "インプットファイルに種スモグルが含まれていません。マップセクションを見直してください。\n";
    exit(1);
  }

  if (map_vec != order_vec) {
    std::cout << "エラー：\n"
         << "マップセクションのモンスター数とオーダーセクションのモンスターが異なります\n";
    exit(1);
  } else if (map_vec != monster_vec) {
    std::cout << "エラー：\n"
              << "マップセクションのモンスター数とモンスターセクションのモンスターが異なります\n";
    exit(1);
  } else if (order_vec != monster_vec) {
    std::cout << "エラー：\n"
              << "オーダーセクションのモンスター数とモンスターセクションのモンスターが異なります\n";
    exit(1);
  }


  // まとめて情報を表示
  numberOfSummon = map_vec.size();
  std::cout << "scs :: 試行回数は" << N_ << "回です\n"
            << "scs :: ターン数は" << T_ << "ターンです\n"
            << "scs :: モンスター数は" << numberOfSummon << "です\n"
            << "\nscs :: インプットファイル[ "
            << filename
            << " ]を正常に読み込みました\n\n";
}

void Input::DeleteComment(string &buf) {
  regex_replace(buf, regex("#.*"), "");
}

void Input::DeleteHeadSpace(string &buf) {
  size_t pos;
  while ((pos = buf.find_first_of(" 　\t")) == 0) {
    buf.erase(buf.begin());
    if (buf.empty()) break;
  }
}

void Input::DeleteSpace(string &buf) {
  size_t pos;
  while ((pos = buf.find_first_of(" 　\t")) != string::npos) {
    buf.erase(pos, 1);
  }
}
