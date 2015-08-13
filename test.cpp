#include <cstdlib>
#include <cstdio>
#include <chrono>
#include <vector>
#include <string>
#include <iostream>
#include "unit.h"
#include "input.h"
#include "map.h"
#include "status.h"

int main(void) {
  setbuf(stdout, NULL);
  std::string filename("../build/ocha.txt");
  Input imp(filename);

  FELLOWVECTOR u_fellow;
  ENEMYVECTOR u_enemy;
  Map mp;
  // メモリー確保
  u_fellow.reserve(10);
  u_enemy.reserve(400);

  const auto t_start = std::chrono::system_clock::now();
  switch ( imp.MODE_ ) {
  case Input::EFFICIENCY:
    mp.EfficiencyMode(&u_fellow, &u_enemy, &imp);
    break;
  case Input::DURABILITY:
    mp.DurabilityMode(&u_fellow, &u_enemy, &imp);
    break;
  case Input::SAFETY:
    mp.SafetyMode(&u_fellow, &u_enemy, &imp);
    break;
  case Input::DEBUG:
    mp.DebugMode(&u_fellow, &u_enemy, &imp);
    break;
  }
  const auto t_end = std::chrono::system_clock::now();
  const std::chrono::duration<double> elapsed = t_end - t_start;
  printf("経過時間:\t%.2f[s]\n", elapsed.count());


  return 0;
}
