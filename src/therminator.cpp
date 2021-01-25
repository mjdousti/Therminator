/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#include "headers/device.hpp"
#include "headers/general.hpp"
#include "headers/model.hpp"
#include "headers/parser.hpp"

void printUsage(char *argv0) {
  cerr << "usage: therminator -d <file> -p <file> -o <file> [-t]" << endl;
  cerr << "Therminator v2: A fast thermal simulator for portable devices"
       << endl;

  cerr << " -d <file>\tInput design specification file" << endl;
  cerr << " -p <file>\tInput power trace file" << endl;
  cerr << " -o <file>\tOutput file" << endl;
  cerr << " -t\t\tTransient analysis" << endl;
  cerr << " -e\t\tExport internal matrices and vectors" << endl;
  cerr << " -h\t\tShows this help menu" << endl;
  exit(-1);
}

string FLAGS_output_file;
string FLAGS_design_file;
string FLAGS_trace_file;
bool FLAGS_transient;
bool FLAGS_export;

void parseCMD(int argc, char **argv) {
  bool flag_designfile = false;
  bool flag_tracefile = false;
  bool flag_outfile = false;

  if (argc <= 1 || argv[1] == string("-h") || argv[1] == string("-help") ||
      argv[1] == string("--help")) {
    printUsage(argv[0]);
  }

  FLAGS_transient = false; // default is steady-state analysis
  FLAGS_export = false;
  /* First argument is the program name */
  for (int i = 1; i < argc; i++) {
    if (argv[i] == string("-d")) {
      flag_designfile = true;
      i++;
      FLAGS_design_file.assign(argv[i]);
    } else if (argv[i] == string("-p")) {
      flag_tracefile = true;
      i++;
      FLAGS_trace_file.assign(argv[i]);
    } else if (argv[i] == string("-o")) {
      flag_outfile = true;
      i++;
      FLAGS_output_file.assign(argv[i]);
    } else if (argv[i] == string("-t")) {
      FLAGS_transient = true;
    } else if (argv[i] == string("-e")) {
      FLAGS_export = true;
    } else {
      cerr << "Option " << argv[i] << " is not supported." << endl;
      printUsage(argv[0]);
    }
  }
  if (!flag_designfile || !flag_tracefile || !flag_outfile) {
    printUsage(argv[0]);
  }
}

Model *initModel() {
  Device *device = Parser::parseDevice(FLAGS_design_file, FLAGS_trace_file);
  ASSERT(device != NULL, "Failed to create a device intance.\n");

  Model *model = new Model(device, FLAGS_transient);
  model->makeResistanceModel();

  if (FLAGS_transient) {
    model->makeCapacitanceModel();
  }

  return model;
}

unsigned int solveInitState(Model *model) {
  auto requiredTempCount = model->read_power();

  ASSERT(requiredTempCount != 0, "Power trace file doesn't contain any value.");

  model->solveSteadyState();
  if (FLAGS_transient) {
    model->printComponentTemp(FLAGS_output_file, 0 /* step number */);
  } else {
    model->printComponentTemp(FLAGS_output_file);
  }

  return requiredTempCount;
}

void exportData(Model *model) {
  if (!FLAGS_export) {
    return;
  }
  model->printGMatrix(FLAGS_output_file + "_export_g_matrix");
  if (FLAGS_transient) {
    model->printInvCVector(FLAGS_output_file + "_export_inv_c");
    model->printAMatrix(FLAGS_output_file + "_export_a_matrix");
  }
  model->printPVector(FLAGS_output_file + "_export_power");
  model->printTVector(FLAGS_output_file + "_export_temperature");
  model->printElementCount(FLAGS_output_file + "_export_count");
}

int main(int argc, char **argv) {
  // Recording the start time of the program
  auto start_time = std::chrono::high_resolution_clock::now();

  /* Parsing the input parameters */
  parseCMD(argc, argv);

  auto model = initModel();
  auto requiredTempCount = solveInitState(model);
  exportData(model);

  unsigned step_no = 1;
  while (FLAGS_transient) {
    auto count = model->read_power();
    if (count == 0) {
      // No more power trace exists.
      break;
    }
    ASSERT(count == requiredTempCount,
           "Expected to get " << requiredTempCount << " power traces, but got "
                              << count << ".\n");
    model->solveTransientState();
    model->printComponentTemp(FLAGS_output_file, step_no++);
  }
  // Recording the end time of the program
  auto end_time = std::chrono::high_resolution_clock::now();

  delete (model);
  // cout << "\nThe program is finished successfully.\n";
  cout << std::fixed << std::setprecision(3)
       << "\nTotal runtime: " << utils::calcElapsedTime(start_time, end_time)
       << " s\n";
  return 0;
}
