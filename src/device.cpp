/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#include "headers/device.hpp"
#include <algorithm>
#include <tuple>
#include <unordered_set>

Device::Device(string s, VALUE l, VALUE w, VALUE h)
    : PhysicalEntity(s, l, w, h, 0, 0, 0) {
  componentCount = 0;
  t_amb = 298.15; // in K equals to 25C
}

void Device::setTemperature(VALUE t) { t_amb = t; }

VALUE Device::getTemperature() { return t_amb; }

void Device::makeComponentsIndex() {
  componentCount = 0;
  Component *c;

  for (list<Component *>::iterator iterator = components.begin();
       iterator != components.end(); iterator++) {
    c = (*iterator);
    componentCount += c->getSubComponentCount();
    for (int i = 0; i < c->getSubComponentCount(); i++) {
      subComponents.push_back(c->getSubComponent(i));
    }
  }
}

int Device::getComponentCount() { return componentCount; }
int Device::getSubComponentCount() { return subComponents.size(); }

void Device::fillGaps() {
  Component *c1, *c2, *paste;
  int i = 0;

  VALUE topDistance, bottomDistance;
  std::ostringstream pasteName;
  for (list<Component *>::iterator it1 = components.begin();
       it1 != components.end(); it1++) {
    c1 = (*it1);
    if (c1->hasToBeFilled()) {
      topDistance = bottomDistance = std::numeric_limits<VALUE>::max();
      for (list<Component *>::iterator it2 = components.begin();
           it2 != components.end(); it2++) {
        c2 = (*it2);
        // check to see if c1 and c2 are aligned in the vertical direction
        // below/above each other.
        if (c1 != c2 &&
            ((utils::ge(c1->getX(), c2->getX()) &&
              utils::le(c1->getX() + c1->getLength(),
                        c2->getX() + c2->getLength())) ||
             (utils::le(c1->getX(), c2->getX()) &&
              utils::ge(c1->getX() + c1->getLength(),
                        c2->getX() + c2->getLength()))) &&
            ((utils::ge(c1->getY(), c2->getY()) &&
              utils::le(c1->getY() + c1->getWidth(),
                        c2->getY() + c2->getWidth())) ||
             (utils::le(c1->getY(), c2->getY()) &&
              utils::ge(c1->getY() + c1->getWidth(),
                        c2->getY() + c2->getWidth())))) {
          // finding the required height of the material to fill the gap
          if (c2->getZ() > c1->getZ() &&
              utils::ge(c2->getZ(), c1->getZ() + c1->getHeight()) &&
              topDistance > abs(c2->getZ() - c1->getZ() - c1->getHeight())) {
            topDistance = c2->getZ() - c1->getZ() - c1->getHeight();
          } else if (c1->getZ() > c2->getZ() &&
                     utils::ge(c1->getZ(), c2->getZ() + c2->getHeight()) &&
                     bottomDistance >
                         abs(c1->getZ() - c2->getZ() - c2->getHeight())) {
            bottomDistance = abs(c1->getZ() - c2->getZ() - c2->getHeight());
          }
        }
      }
      if (utils::eq(bottomDistance, 0) &&
          utils::eq(topDistance, 0)) { // no filling material is required
        continue;
      }
      // clearing the previous name
      pasteName.str("");
      pasteName.clear();
      pasteName << c1->getFillType()->getName() << i;

      if ((utils::eq(bottomDistance, 0) || bottomDistance > topDistance) &&
          !utils::eq(topDistance, 0)) {
        paste = new Component(pasteName.str(), c1->getLength(), c1->getWidth(),
                              topDistance, c1->getX(), c1->getY(),
                              c1->getZ() + c1->getHeight());
      } else { // paste at the bottom, i.e., chip is assembled below the PCB
        paste = new Component(pasteName.str(), c1->getLength(), c1->getWidth(),
                              bottomDistance, c1->getX(), c1->getY(),
                              c1->getZ() - bottomDistance);
      }

      paste->setFloorplan(NULL);
      paste->setLateralConnectiviy(true);
      paste->setMaterial(c1->getFillType());
      paste->setPowerGen(false);
      if (c1->hasFloorPlan()) {
        paste->setFloorplan(c1->getFloorplan());
      } else {
        paste->setResolution(c1->getResolution().length,
                             c1->getResolution().width,
                             c1->getResolution().height);
      }
      // adding the component to the device
      addCompopnent(paste);
      // this counter is used in naming the thermal paste components
      i++;
    }
  }
}
void Device::setPowerTraceFile(string s) { power_trace_file = s; }

string Device::getPowerTraceFile() { return power_trace_file; }

void Device::addMaterial(Material *m) {
  if (materials.find(m->getName()) != materials.end()) {
    cerr << "Material " << m->getName() << " is defined more than once.\n";
  }
  materials.insert(std::pair<string, Material *>(m->getName(), m));
}

Material *Device::getMaterial(string name) {
  if (materials.find(name) != materials.end()) {
    return materials.at(name);
  } else {
    cerr << "Material " << name << " not found.\n";
    return NULL;
  }
}

void Device::addFloorplan(Floorplan *f) {
  if (floorplans.find(f->getName()) != floorplans.end()) {
    cerr << "Floorplan " << f->getName() << " is defined more than once.\n";
  }
  floorplans.insert(std::pair<string, Floorplan *>(f->getName(), f));
}

Floorplan *Device::getFloorplan(string name) {
  if (floorplans.find(name) == floorplans.end())
    return NULL;
  else
    return floorplans.at(name);
}

void Device::addCompopnent(Component *c) {
  if (getComponent(c->getName()) != NULL) {
    cerr << "Component " << c->getName() << " is defined more than once.\n";
  }
  ASSERT(utils::le(c->getX() + c->getLength(), getLength()) &&
             utils::le(c->getY() + c->getWidth(), getWidth()) &&
             utils::le(c->getZ() + c->getHeight(), getHeight()),
         c->getName() << " does not fit in " << getName() << " boundaries.");

  components.push_back(c);
}

vector<SubComponent *> Device::getSubComponents() { return subComponents; }

SubComponent *Device::getSubComponent(int i) { return subComponents[i]; }

Component *Device::getComponent(string name) {
  for (list<Component *>::iterator iterator = components.begin();
       iterator != components.end(); iterator++) {
    if ((*iterator)->getName().compare(name) == 0) {
      return (*iterator);
    }
  }
  return NULL;
}

list<Component *> Device::getComponents() { return components; }

void Device::componentsOverlap() {
  Component *c1, *c2;
  for (list<Component *>::iterator it1 = components.begin();
       it1 != components.end(); it1++) {
    c1 = (*it1);
    list<Component *>::iterator it1_next = it1;
    it1_next++;
    for (list<Component *>::iterator it2 = it1_next; it2 != components.end();
         it2++) {
      c2 = (*it2);
      if ((utils::le(c1->getX(), c2->getX()) &&
           utils::greater(c1->getX() + c1->getLength(), c2->getX()) &&
           utils::le(c1->getY(), c2->getY()) &&
           utils::greater(c1->getY() + c1->getWidth(), c2->getY()) &&
           utils::le(c1->getZ(), c2->getZ()) &&
           utils::greater(c1->getZ() + c1->getHeight(), c2->getZ())) ||

          (utils::ge(c1->getX(), c2->getX()) &&
           utils::greater(c2->getX() + c2->getLength(), c1->getX()) &&
           utils::le(c1->getY(), c2->getY()) &&
           utils::greater(c1->getY() + c1->getWidth(), c2->getY()) &&
           utils::le(c1->getZ(), c2->getZ()) &&
           utils::greater(c1->getZ() + c1->getHeight(), c2->getZ())) ||

          (utils::le(c1->getX(), c2->getX()) &&
           utils::greater(c1->getX() + c1->getLength(), c2->getX()) &&
           utils::ge(c1->getY(), c2->getY()) &&
           utils::greater(c2->getY() + c2->getWidth(), c1->getY()) &&
           utils::le(c1->getZ(), c2->getZ()) &&
           utils::greater(c1->getZ() + c1->getHeight(), c2->getZ())) ||

          (utils::ge(c1->getX(), c2->getX()) &&
           utils::greater(c2->getX() + c2->getLength(), c1->getX()) &&
           utils::ge(c1->getY(), c2->getY()) &&
           utils::greater(c2->getY() + c2->getWidth(), c1->getY()) &&
           utils::le(c1->getZ(), c2->getZ()) &&
           utils::greater(c1->getZ() + c1->getHeight(), c2->getZ())) ||

          (utils::le(c1->getX(), c2->getX()) &&
           utils::greater(c1->getX() + c1->getLength(), c2->getX()) &&
           utils::le(c1->getY(), c2->getY()) &&
           utils::greater(c1->getY() + c1->getWidth(), c2->getY()) &&
           utils::ge(c1->getZ(), c2->getZ()) &&
           utils::greater(c2->getZ() + c2->getHeight(), c1->getZ())) ||

          (utils::ge(c1->getX(), c2->getX()) &&
           utils::greater(c2->getX() + c2->getLength(), c1->getX()) &&
           utils::le(c1->getY(), c2->getY()) &&
           utils::greater(c1->getY() + c1->getWidth(), c2->getY()) &&
           utils::ge(c1->getZ(), c2->getZ()) &&
           utils::greater(c2->getZ() + c2->getHeight(), c1->getZ())) ||

          (utils::le(c1->getX(), c2->getX()) &&
           utils::greater(c1->getX() + c1->getLength(), c2->getX()) &&
           utils::ge(c1->getY(), c2->getY()) &&
           utils::greater(c2->getY() + c2->getWidth(), c1->getY()) &&
           utils::ge(c1->getZ(), c2->getZ()) &&
           utils::greater(c2->getZ() + c2->getHeight(), c1->getZ())) ||

          (utils::ge(c1->getX(), c2->getX()) &&
           utils::greater(c2->getX() + c2->getLength(), c1->getX()) &&
           utils::ge(c1->getY(), c2->getY()) &&
           utils::greater(c2->getY() + c2->getWidth(), c1->getY()) &&
           utils::ge(c1->getZ(), c2->getZ()) &&
           utils::greater(c2->getZ() + c2->getHeight(), c1->getZ()))) {
        cerr << "Component " << c1->getName() << " and " << c2->getName()
             << " overlap.\n";
        c1->printDetails();
        cout << "\n";
        c2->printDetails();
        cout << "\n";
        // It's a fatal error
        exit(-1);
      }
    }
  }
}

Device::~Device() {
  for (auto iterator = materials.begin(); iterator != materials.end();
       ++iterator) {
    delete (iterator->second);
  }
  for (auto iterator = floorplans.begin(); iterator != floorplans.end();
       ++iterator) {
    delete (iterator->second);
  }
  for (auto iterator = components.begin(); iterator != components.end();
       ++iterator) {
    delete (*iterator);
  }
}
