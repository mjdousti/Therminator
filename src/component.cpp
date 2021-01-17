/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#include "headers/component.h"

Component::Component(string s, VALUE l, VALUE w, VALUE h, VALUE x, VALUE y,
                     VALUE z)
    : PhysicalEntity(s, l, w, h, x, y, z) {
  floorplan = NULL;
  lateral_connectivity = false;
  power_gen = false;
  fill = false;
  fill_type = NULL;
  material = NULL;
  // The -1 values are used to make sure that both resolution and floorplan is
  // not used
  resolution.length = -1;
  resolution.width = -1;
  resolution.height = -1;
}
void Component::setMaterial(Material *m) { material = m; }

bool Component::hasFloorPlan() {
  if (floorplan == NULL)
    return false;
  else
    return true;
}
Floorplan *Component::getFloorplan() { return floorplan; }
void Component::setFloorplan(Floorplan *f) {
  ASSERT(resolution.length == -1 && resolution.width == -1 &&
             resolution.height == -1,
         "Component " << getName()
                      << " can have either resolution or floorplan.");

  floorplan = f;

  if (floorplan == NULL)
    return;

  std::ostringstream subComponentName;
  for (int i = 0; i < floorplan->getSubComponentCount(); i++) {
    SubComponent *temp = floorplan->getSubComponent(i);
    // Assertion to make sure that the floorplan is valid for the component
    ASSERT(temp->getX() + temp->getLength() <= getLength() &&
               temp->getY() + temp->getWidth() <= getWidth(),
           "The tile " << temp->getName() << " in the floorplan "
                       << floorplan->getName()
                       << " does not fit in the component " << getName()
                       << ".");
  subComponentName.str("");
  subComponentName.clear();
  subComponentName << getName() << "-" << temp->getName();
  // Note: the X & Y are relative values in the floorplan assuming that the
  // bottom-left corner is designated as (0,0)
  SubComponent *sc = new SubComponent(
      subComponentName.str(), temp->getLength(), temp->getWidth(), getHeight(),
      getX() + temp->getX(), getY() + temp->getY(), getZ(), this);
  subComponents.push_back(sc);
}
}
void Component::setPowerGen(bool b) { power_gen = b; }
void Component::setLateralConnectiviy(bool b) { lateral_connectivity = b; }

void Component::setResolution(int l, int w, int h) {
  ASSERT(floorplan == NULL, "Component "
                                << getName()
                                << " can have either resolution or floorplan.");
  ASSERT(l >= 1 && w >= 1 && h >= 1,
         "Invalid resolution for component " << getName() << ".");

  resolution.length = l;
  resolution.width = w;
  resolution.height = h;

  std::ostringstream subComponentName;
  VALUE length = getLength();
  VALUE width = getWidth();
  VALUE height = getHeight();

  for (int k = 0; k < resolution.height; k++) {
    for (int i = 0; i < resolution.length; i++) {
      for (int j = 0; j < resolution.width; j++) {
        subComponentName.str("");
        subComponentName.clear();
        subComponentName << getName() << "(" << i << "," << j << "," << k
                         << ")";
        SubComponent *sc = new SubComponent(
            subComponentName.str(), length / resolution.length,
            width / resolution.width, height / resolution.height,
            getX() + i * (length / resolution.length),
            getY() + j * (width / resolution.width),
            getZ() + k * (height / resolution.height), this);
        // The first component is primary and designates the Component
        if (i == 0 && j == 0 && k == 0)
          sc->setPrimary();
        subComponents.push_back(sc);
      }
    }
  }
}

SubComponent *Component::getSubComponent(int i) { return subComponents[i]; }

Resolution Component::getResolution() { return resolution; }

int Component::getSubComponentCount() {
  if (floorplan == NULL && resolution.length != -1 && resolution.width != -1 &&
      resolution.height != -1)
    return resolution.length * resolution.width * resolution.height;
  else if (floorplan != NULL && resolution.length == -1 &&
           resolution.width == -1 && resolution.height == -1)
    return floorplan->getSubComponentCount();
  else {
    cerr << "Component " << getName()
         << " can have either resolution or floorplan.\n";
    exit(-1);
  }
}
Material *Component::getMaterial() { return material; }
/**
 * Checks if the component generates power.
 *  @return If the component generates power
 */
bool Component::isPowerGen() { return power_gen; }

bool Component::isLaterallyConnected() { return lateral_connectivity; }
void Component::setFill(bool b) { fill = b; }
bool Component::hasToBeFilled() { return fill; }
/**
 * Sets the material type
 * @param m Material type
 */
void Component::setFillType(Material *m) {
  // Just an assertion
  ASSERT(m != NULL, "The fill type material is not valid.");
  fill_type = m;
}

Material *Component::getFillType() {
  ASSERT(fill_type != NULL, "No fill type is set for " << getName());
  return fill_type;
}

void Component::printDetails() {
  cout << "Component Name: " << getName() << "\n";
  cout << "Material type: " << getMaterial()->getName() << "\n";
  cout << "Position: (" << getX() << "," << getY() << "," << getZ() << ")\n";
  cout << "Size: (" << getLength() << "," << getWidth() << "," << getHeight()
       << ")\n";
  cout << "Resolution: " << getResolution().length << "*"
       << getResolution().width << "*" << getResolution().height << "\n";
  if (isPowerGen()) {
    cout << "Generates power: Yes\n";
  } else {
    cout << "Generates power: No\n";
  }
  if (isLaterallyConnected()) {
    cout << "Laterally dissipates heat: Yes\n";
  } else {
    cout << "Laterally dissipates heat: No\n";
  }
}

Component::~Component() {
  for (vector<SubComponent *>::iterator iterator = subComponents.begin();
       iterator != subComponents.end(); iterator++) {
    delete (*iterator);
  }
}
