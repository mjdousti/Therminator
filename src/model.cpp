/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#include "headers/model.h"
#include "headers/neighbor_components.h"
#include "headers/solver.h"

Model::Model(Device *d, bool isTransient) {
  pwr_consumers_cnt = 0;
  transient_ = isTransient;
  // P & G matrices should be made later
  t_vector_made_ = false;
  p_vector_made_ = false;
  g_matrix_made_ = false;
  c_vector_made_ = false;

  device = d;
  elements_no_ = device->getComponentCount();
  cout << "Component count: " << elements_no_ << "\n";
  p_vector_.resize(elements_no_);
  // Allocating space for the temperature vector
  t_vector_.resize(elements_no_);

  g_matrix_.resize(elements_no_, elements_no_);
  amb_vector_.resize(elements_no_);

  if (transient_) {
    a_matrix_.resize(elements_no_, elements_no_);
    inv_c_.resize(elements_no_);
  }
}

void Model::initPowerVector() { p_vector_ = amb_vector_; }

bool Model::isComment(string s) {
  // skipping from comment
  size_t found;
  found = s.find_first_not_of(" \t");

  if (found == string::npos ||
      s[found] == '#') { // it either consists of whitespace or comment
    return true;
  } else {
    return false;
  }
}

void Model::preparePVector() {
  string temp;
  list<string> tokenList;
  vector<SubComponent *> subComponents = device->getSubComponents();
  SubComponent *sc;
  int i = 0;
  pwr_consumers_cnt = 0;

  // Storing the device subcomponents order in a map, i.e.,
  // powerMappingDeviceOrder in order to access them while filling the P vector
  for (vector<SubComponent *>::iterator it1 = subComponents.begin();
       it1 != subComponents.end(); it1++) {
    sc = (*it1);
    // avoid using (i,j) concatenated with the names in the high-res components
    if (sc->isPrimary()) {
      powerMappingDeviceOrder.insert(
          std::pair<string, int>(sc->getComponent()->getName(), i));
    } else {
      powerMappingDeviceOrder.insert(std::pair<string, int>(sc->getName(), i));
    }
    if (sc->getComponent()->isPowerGen() &&
        (sc->getComponent()->hasFloorPlan() || sc->isPrimary())) {
      pwr_consumers_cnt++;
    }
    i++;
  }
  // Opening the power trace file for reading
  string powerTraceFileAddr = device->getPowerTraceFile();

  powerTraceFile.open(powerTraceFileAddr.c_str(), std::ifstream::in);

  ASSERT(powerTraceFile.is_open(), "Could not open "
                                       << powerTraceFileAddr
                                       << " for parsing as the power trace.");

  do {
    getline(powerTraceFile, temp);
    boost::algorithm::trim(temp);
  } while (isComment(temp) && powerTraceFile.eof() == false);

  ASSERT(!powerTraceFile.eof(), "The file does not contain valid power trace.");

  split(tokenList, temp, boost::algorithm::is_any_of("\t "),
        boost::algorithm::token_compress_on);
  i = 0;
  for (string token : tokenList) {
    powerMappingTraceOrder[i] = std::move(token);
    i++;
  }
  // Check the number of titles and the power consumer subcomponents match.
  ASSERT(pwr_consumers_cnt == i,
         "The number of (sub)component power trace in "
             << powerTraceFileAddr << " do not match with the XML description."
             << " The expected sub(component) count was " << pwr_consumers_cnt
             << " but " << i << " subcomponents are provided.");
}

unsigned Model::read_power() {
  p_vector_made_ = true;
  t_vector_made_ = false;

  if (!powerTraceFile.is_open()) {
    initPowerVector();
    preparePVector();
  }

  string temp;
  do {
    if (powerTraceFile.eof() == false) {
      getline(powerTraceFile, temp);

      boost::algorithm::trim(temp);
    } else {
      powerTraceFile.close();
      return 0;
    }
  } while (isComment(temp));

  // A p_vector_ is required to build proper right_hand side
  initPowerVector();

  list<string> powerList;
  split(powerList, temp, boost::algorithm::is_any_of("\t "),
        boost::algorithm::token_compress_on);
  auto i = 0;
  // Adding the power consumption of subcomponents to the P vector
  for (const auto &power : powerList) {
    if (powerMappingDeviceOrder.find(powerMappingTraceOrder[i]) ==
        powerMappingDeviceOrder.end()) {
      cerr
          << "Component " << powerMappingTraceOrder[i]
          << " is listed in the trace file but not found in the design file.\n";
      powerTraceFile.close();
      exit(-1);
    }
    auto subComponentIndex = powerMappingDeviceOrder[powerMappingTraceOrder[i]];

    // If the subcomponent has a resolution, assume the uniform heat
    // distribution for that component
    if (device->getSubComponent(subComponentIndex)->isPrimary()) {
      for (int j = subComponentIndex;
           j < subComponentIndex + device->getSubComponent(subComponentIndex)
                                       ->getComponent()
                                       ->getSubComponentCount();
           j++) {
        p_vector_(j) +=
            atof(power.c_str()) / device->getSubComponent(subComponentIndex)
                                      ->getComponent()
                                      ->getSubComponentCount();
      }
    } else {
      p_vector_(subComponentIndex) = atof(power.c_str());
    }
    i++;
  }

  // Check the power values and the power consumer subcomponents match.
  ASSERT(pwr_consumers_cnt == i,
         "The number of (sub)component power trace in "
             << powerTraceFileAddr << " do not match with the XML description."
             << " The expected sub(component) count was " << pwr_consumers_cnt
             << " but " << i << " subcomponents are provided.");

  return powerList.size();
}

void Model::makeCapacitanceModel() {
  int i = 0;
  SubComponent *sc;
  vector<SubComponent *> subComponents = device->getSubComponents();
  ASSERT(g_matrix_made_, "G matrix is not initialized.");

  VALUE cap, area;
  VALUE c_convec_per_area = 39e3;
  for (vector<SubComponent *>::iterator it1 = subComponents.begin();
       it1 != subComponents.end(); it1++) {
    sc = (*it1);

    cap = RCutils::calcSubComponentCapacitance(sc);

    if (RCutils::touchesAirFromTopBot(sc, device)) {
      area = sc->getLength() * sc->getWidth();
      cap += C_FACTOR * c_convec_per_area * area;
    }
    if (RCutils::touchesAirInXDir(sc, device)) {
      area = sc->getWidth() * sc->getHeight();
      cap += C_FACTOR * c_convec_per_area * area;
    }
    if (RCutils::touchesAirInYDir(sc, device)) {
      area = sc->getLength() * sc->getHeight();
      cap += C_FACTOR * c_convec_per_area * area;
    }

    inv_c_.diagonal()[i] = 1 / cap;
    i++;
  }
  // A = C^-1 * G
  a_matrix_ = inv_c_ * g_matrix_;
  c_vector_made_ = true;
}

void Model::makeResistanceModel() {
  int i = 0, j = 0;

  SubComponent *sc1, *sc2;
  vector<SubComponent *> subComponents = device->getSubComponents();
  vector<Eigen::Triplet<VALUE>> triplet;
  vector<VALUE> diagonal(subComponents.size(), 0.0);
  for (vector<SubComponent *>::iterator it1 = subComponents.begin();
       it1 != subComponents.end(); it1++) {
    sc1 = (*it1);
    vector<SubComponent *>::iterator it1_next = it1;
    it1_next++;
    j = i;
    for (vector<SubComponent *>::iterator it2 = it1_next;
         it2 != subComponents.end(); it2++) {
      j++;
      sc2 = (*it2);
      auto commonConductance = RCutils::calcCommonConductance(sc1, sc2);
      if (utils::neq(commonConductance, 0)) {
        triplet.push_back(Eigen::Triplet<VALUE>(i, j, -commonConductance));
        triplet.push_back(Eigen::Triplet<VALUE>(j, i, -commonConductance));
        diagonal[i] += commonConductance;
        diagonal[j] += commonConductance;
      }
    }
    // Adding the conductances to air for items in the borders of the device
    auto K1 = RCutils::calcConductanceToAmbient(sc1, device);
    if (utils::neq(K1, 0)) {
      diagonal[i] += K1;
      amb_vector_(i) = device->getTemperature() * K1;
    } else {
      amb_vector_(i) = 0;
    }
    triplet.push_back(Eigen::Triplet<VALUE>(i, i, diagonal[i]));
    i++;
  }
  g_matrix_.setFromTriplets(triplet.begin(), triplet.end());
  g_matrix_.makeCompressed();
  g_matrix_made_ = true;
}

void Model::solveSteadyState() {
  auto start_time = std::chrono::high_resolution_clock::now();

  ASSERT(p_vector_made_, "P vector is not initialized.\n");
  ASSERT(g_matrix_made_, "G matrix is not initialized.\n");

  cout << "\nCalling the steady solver...\n";
#if USE_GPU == 1
  //solver::gpuSolveDenseSteady(g_matrix_, p_vector_, t_vector_, false);
  solver::gpuSolveSparseSteady(g_matrix_, p_vector_, t_vector_);
#else
  solver::cpuSolveSteady(g_matrix_, p_vector_, t_vector_);
  // hotspot::cpuSolveSteady(g_matrix_, elements_no_, p_vector_, t_vector_);
#endif

  t_vector_made_ = true;
  auto end_time = std::chrono::high_resolution_clock::now();
  cout << std::fixed << std::setprecision(3)
       << "Runtime: " << utils::calcElapsedTime(start_time, end_time) << " s\n";
}

void Model::solveTransientState() {
#if USE_GPU == 1
  solver::gpuSolveTransient();
#else
  auto start_time = std::chrono::high_resolution_clock::now();

  ASSERT(p_vector_made_, "P vector is not initialized.\n");
  ASSERT(g_matrix_made_, "G matrix is not initialized.\n");
  ASSERT(t_vector_[0] != 0, "No initial power trace is provided.\n");

  cout << "\nCalling the transient solver...\n";
  solver::cpuSolveTransient(a_matrix_, inv_c_, p_vector_, t_vector_,
                            0.0 /* time_start */, 1.0 /* time_end */);
  t_vector_made_ = true;
  auto end_time = std::chrono::high_resolution_clock::now();
  cout << std::fixed << std::setprecision(3)
       << "Runtime: " << utils::calcElapsedTime(start_time, end_time) << " s\n";
#endif
}

void Model::printComponentTemp(string file_output, unsigned step_no) {
  this->printComponentTemp(file_output + "_" + std::to_string(step_no));
}

void Model::printComponentTemp(string file_output) {
  if (!t_vector_made_) {
    cerr << "The temperature is not calculated.\n";
    return;
  }
  VALUE temp_avg = 0.0;
  VALUE temp_max = 0.0;
  int count = 0;
  int temp_width = 0;
  int blk_size = 0;
  int z_index = 0;

  SubComponent *sc = NULL;
  std::ofstream temp_out(file_output, std::ofstream::out);
  for (int j = 0; j < elements_no_; j++) {
    sc = device->getSubComponent(j);
    if (sc->isPrimary()) {
      // Do NOT put this line above the previous if statement
      count = 1;

      temp_width =
          device->getSubComponent(j)->getComponent()->getResolution().width;
      blk_size =
          (device->getSubComponent(j)->getComponent()->getResolution().length) *
          temp_width;
      z_index =
          device->getSubComponent(j)->getComponent()->getResolution().height;

      temp_out << device->getSubComponent(j)->getComponent()->getName() << ":"
               << "\n";
      temp_out << "z=" << z_index << "\n";

      temp_out << std::fixed << std::setprecision(1)
               << utils::KtoC(t_vector_[j]) << "\t";
      temp_avg = t_vector_[j];
      temp_max = t_vector_[j];

    } else {
      count++;

      temp_avg += t_vector_[j];
      temp_max = max(t_vector_[j], temp_max);

      temp_out << std::fixed << std::setprecision(1)
               << utils::KtoC(t_vector_[j]) << "\t";
    }
    if (count % temp_width == 0) {
      temp_out << "\n";
    }
    if (count % (blk_size) == 0 && z_index > 1) {
      z_index--;
      temp_out << "z=" << z_index << "\n";
    }
  }
  temp_out.close();
}

void Model::printSubComponentTemp() {
  SubComponent *sc;
  if (!t_vector_made_) {
    cerr << "The temperature is not calculated.\n";
    return;
  }
  for (int j = 0; j < elements_no_; ++j) {
    sc = device->getSubComponent(j);
    cout << sc->getName() << ":\t" << utils::KtoC(t_vector_[j]) << " C\n";
  }
}

void Model::printElementCount(string file_output) {
  std::ofstream temp_out(file_output, std::ofstream::out);
  temp_out << elements_no_ << "\n";
  temp_out.close();
}

void Model::printTVector(string file_output) {
  ASSERT(t_vector_made_, "T vector is not initialized.");
  utils::dumpVector(t_vector_, file_output);
}

void Model::printGMatrix(string file_output) {
  ASSERT(g_matrix_made_, "G matrix is not initialized.");
  utils::dumpMatrix(g_matrix_, file_output);
}

void Model::printAMatrix(string file_output) {
  ASSERT(g_matrix_made_, "G matrix is not initialized.");
  utils::dumpMatrix(a_matrix_, file_output);
}

void Model::printInvCVector(string file_output) {
  ASSERT(c_vector_made_, "C vector is not initialized.");
  utils::dumpVector(inv_c_, file_output);
}

void Model::printPVector(string file_output) {
  ASSERT(p_vector_made_, "P vector is not initialized.");
  utils::dumpVector(p_vector_, file_output);
}

Model::~Model() {
  delete (device);
  powerTraceFile.close();
}
