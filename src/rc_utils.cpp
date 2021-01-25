/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */
 
#include "headers/rc_utils.hpp"
#include <omp.h>

VALUE RCutils::calcThermalConductivity(VALUE k, VALUE thickness,
                                        VALUE area) {
  ASSERT(thickness != 0, "The thickness of an element cannot be zero.");
  return k * area / thickness;
}

VALUE RCutils::calcSubComponentCapacitance(SubComponent *sc) {
  auto volume = sc->getLength() * sc->getWidth() * sc->getHeight();

  if (volume == 0)
    cout << sc->getName() << " has nil volume.\n";

  else if (sc->getComponent()->getMaterial()->getSpecificHeat() == 0)
    cout << sc->getName() << " has nil specific heat.\n";

  else if (sc->getComponent()->getMaterial()->getDensity() == 0)
    cout << sc->getName() << " has nil density.\n";

  return C_FACTOR * sc->getComponent()->getMaterial()->getSpecificHeat() *
         sc->getComponent()->getMaterial()->getDensity() * volume;
}

VALUE RCutils::calcAmbientResistance(VALUE h, VALUE area) {
  return 1 / (h * area);
}

VALUE RCutils::overallParallelConductivity(VALUE k1, VALUE k2) {
  return (k1 * k2) / (k1 + k2);
}

bool RCutils::touchesAirInXDir(SubComponent *sc, Device *device) {
  if (utils::eq(sc->getX(), device->getX()) ||
      utils::eq(sc->getX() + sc->getLength(),
                device->getX() + device->getLength()))
    return true;
  else
    return false;
}

bool RCutils::touchesAirInYDir(SubComponent *sc, Device *device) {
  if (utils::eq(sc->getY(), device->getY()) ||
      utils::eq(sc->getY() + sc->getWidth(),
                device->getY() + device->getWidth())) {
    return true;
  } else {
    return false;
  }
}

bool RCutils::touchesAirFromTopBot(SubComponent *sc, Device *device) {
  if (utils::eq(sc->getZ(), device->getZ()) ||
      utils::eq(sc->getZ() + sc->getHeight(),
                device->getZ() + device->getHeight()))
    return true;
  else
    return false;
}

VALUE RCutils::calcConductanceToAmbient(SubComponent *sc, Device *device) {
  VALUE commonArea;
  VALUE t1 = 0.0;
  VALUE k = 0.0;
  VALUE Kx = 0.0, Ky = 0.0, Kz = 0.0, Rx, Ry, Rz;
  // Unit: W/m^2/K; Source:
  // <http://www.engineeringtoolbox.com/convective-heat-transfer-d_430.html>
  VALUE h = 10 * 1.15;
  if (touchesAirFromTopBot(sc, device)) { // Touches air from top or bottom
    t1 = sc->getHeight() / 2;
    commonArea = sc->getLength() * sc->getWidth();

    k = sc->getComponent()->getMaterial()->getNormalConductivity();

    Kz = RCutils::calcThermalConductivity(k, t1, commonArea);
    Rz = RCutils::calcAmbientResistance(h, commonArea);
    Kz = RCutils::overallParallelConductivity(Kz, 1 / Rz);
  }
  if (touchesAirInXDir(sc, device)) { // Touches air from the X side
    t1 = sc->getLength() / 2;
    commonArea = sc->getWidth() * sc->getHeight();

    // Setting the k1 value to the proper value if the planar conductivity
    // differs from the normal conductivity
    if (sc->getComponent()->getMaterial()->hasPlanarConductivity()) {
      k = sc->getComponent()->getMaterial()->getPlanarConductivity();
    } else {
      k = sc->getComponent()->getMaterial()->getNormalConductivity();
    }

    Kx = RCutils::calcThermalConductivity(k, t1, commonArea);
    Rx = RCutils::calcAmbientResistance(h, commonArea);
    Kx = RCutils::overallParallelConductivity(Kx, 1 / Rx);
  }
  if (touchesAirInYDir(sc, device)) { // Touches air from the Y side

    t1 = sc->getWidth() / 2;
    commonArea = sc->getLength() * sc->getHeight();

    // Setting the k1 value to the proper value if the planar conductivity
    // differs from the normal conductivity
    if (sc->getComponent()->getMaterial()->hasPlanarConductivity()) {
      k = sc->getComponent()->getMaterial()->getPlanarConductivity();
    } else {
      k = sc->getComponent()->getMaterial()->getNormalConductivity();
    }

    Ky = RCutils::calcThermalConductivity(k, t1, commonArea);
    Ry = RCutils::calcAmbientResistance(h, commonArea);
    Ky = RCutils::overallParallelConductivity(Ky, 1 / Ry);
  }
  return Kx + Ky + Kz;
}

VALUE RCutils::calcCommonConductance(SubComponent *sc1, SubComponent *sc2) {
  VALUE commonArea;
  VALUE t1 = 0, t2 = 0;
  VALUE k1, k2;

  // common area in the Y & Z planes
  VALUE commonX, commonY, commonZ;

  commonX =
      min(sc1->getX() + sc1->getLength(), sc2->getX() + sc2->getLength()) -
      max(sc1->getX(), sc2->getX());
  commonY = min(sc1->getY() + sc1->getWidth(), sc2->getY() + sc2->getWidth()) -
            max(sc1->getY(), sc2->getY());
  commonZ =
      min(sc1->getZ() + sc1->getHeight(), sc2->getZ() + sc2->getHeight()) -
      max(sc1->getZ(), sc2->getZ());

  if (commonZ > 0 && commonY > 0 &&
      (utils::eq(sc1->getX() + sc1->getLength(), sc2->getX()) ||
       utils::eq(sc2->getX() + sc2->getLength(), sc1->getX()))) {
    commonArea = commonY * commonZ;
    t1 = sc1->getLength() / 2;
    t2 = sc2->getLength() / 2;

    // using planar thermal conductivity if the material has different value for
    // it
    if (sc1->getComponent()->getMaterial()->hasPlanarConductivity()) {
      k1 = sc1->getComponent()->getMaterial()->getPlanarConductivity();
    } else {
      k1 = sc1->getComponent()->getMaterial()->getNormalConductivity();
    }
    if (sc2->getComponent()->getMaterial()->hasPlanarConductivity()) {
      k2 = sc2->getComponent()->getMaterial()->getPlanarConductivity();
    } else {
      k2 = sc2->getComponent()->getMaterial()->getNormalConductivity();
    }
  } else if (commonX > 0 && commonZ > 0 &&
             (utils::eq(sc1->getY() + sc1->getWidth(), sc2->getY()) ||
              utils::eq(sc2->getY() + sc2->getWidth(), sc1->getY()))) {
    commonArea = commonX * commonZ;
    t1 = sc1->getWidth() / 2;
    t2 = sc2->getWidth() / 2;

    // using planar thermal conductivity if the material has different value for
    // it
    if (sc1->getComponent()->getMaterial()->hasPlanarConductivity()) {
      k1 = sc1->getComponent()->getMaterial()->getPlanarConductivity();
    } else {
      k1 = sc1->getComponent()->getMaterial()->getNormalConductivity();
    }
    if (sc2->getComponent()->getMaterial()->hasPlanarConductivity()) {
      k2 = sc2->getComponent()->getMaterial()->getPlanarConductivity();
    } else {
      k2 = sc2->getComponent()->getMaterial()->getNormalConductivity();
    }
  } else if (commonX > 0 && commonY > 0 &&
             (utils::eq(sc1->getZ() + sc1->getHeight(), sc2->getZ()) ||
              utils::eq(sc2->getZ() + sc2->getHeight(), sc1->getZ()))) {
    commonArea = commonX * commonY;
    t1 = sc1->getHeight() / 2;
    t2 = sc2->getHeight() / 2;

    // using normal conductivity since it is in the vertical direction
    k1 = sc1->getComponent()->getMaterial()->getNormalConductivity();
    k2 = sc2->getComponent()->getMaterial()->getNormalConductivity();

  } else {
    commonArea = 0;
    return 0;
  }

  auto K1 = RCutils::calcThermalConductivity(k1, t1, commonArea);
  auto K2 = RCutils::calcThermalConductivity(k2, t2, commonArea);

  return RCutils::overallParallelConductivity(K1, K2);
}
