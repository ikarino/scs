// Copyright 2014 ikarino
#include <boost/program_options.hpp>
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

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
  setbuf(stdout, NULL);
  std::string filename;
  int OpSpeed = 0;
  int OpMode  = -1;

  // オプション処理
  try {
    po::options_description desc("オプションの説明");
    desc.add_options()
        ("help,h", "ヘルプの出力")
        ("version,v", "バージョン情報")
        ("speed,s", po::value<int>() , "等速:1、倍速:2")
        ("mode,m", po::value<int>() , "効率性:0、持続性:1、安定性:2")
        // ("input,i", po::value<std::string>()->value_name("<path>")->default_value("./input.txt"), "インプットファイル");
        ("input,i", po::value<std::string>(), "インプットファイル");
    po::command_line_parser parser(argc, argv);
    parser.options(desc);

    po::variables_map vm;
    po::store(parser.run(), vm);
    po::notify(vm);

    if ( vm.count("help") ) {
      std::cout << "scs [option]...\n"
                << desc << std::endl;
      return 0;
    }
    if ( vm.count("version") ) {
      std::cout << "scs ver 1.0" << std::endl
                << "Copyright (C) 2014 ikarino99" << std::endl
               << "May 28" << std::endl;
      return 0;
    }
    if ( vm.count("input") ) {
      filename = vm["input"].as<std::string>();
    }
    if ( vm.count("speed") ) {
      OpSpeed = vm["speed"].as<int>();
    }
    if ( vm.count("mode") ) {
      OpMode = vm["mode"].as<int>();
    }
  } catch(std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    return 0;
  } catch(...) {
    std::cerr << "Exception of unknown type!\n";
  }
  // ファイル読み込み
  Input imp(filename);
  imp.SetSpeed(OpSpeed);
  imp.SetMode(OpMode);

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
