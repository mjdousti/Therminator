/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#include "headers/parser.hpp"

Device *Parser::parseDevice(const string& design, const string& trace) {
  string name;
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(design.c_str());
  if (!result) {
    cout << "Error in parsing " << design << ".\n";
    cout << "Error description: " << result.description() << ".\n";
    cout << "Error offset: " << result.offset << ".\n";
    return NULL;
  }

  /* Reading device name and its dimensions */
  string deviceName = doc.child("device").attribute("name").value();
  auto l = doc.child("device").child("length").text().as_double();
  auto w = doc.child("device").child("width").text().as_double();
  auto h = doc.child("device").child("height").text().as_double();

  Device *device = new Device(deviceName, l, w, h);
  //    name=doc.child("device").child("power_trace_file").child_value();
  device->setPowerTraceFile(trace);

  auto t_amb = doc.child("device").child("temperature").text().as_double();
  device->setTemperature(t_amb);

  /* Making the material database */
  pugi::xml_node material_nodes = doc.child("device").child("materials");
  for (pugi::xml_node material_node = material_nodes.child("material");
       material_node; material_node = material_node.next_sibling("material")) {
    name = material_node.attribute("name").value();
    auto normal_conductivity =
        material_node.child("normal_conductivity").text().as_double();
    auto planar_conductivity =
        material_node.child("planar_conductivity").text().as_double();
    auto specific_heat =
        material_node.child("specific_heat").text().as_double();
    auto density = material_node.child("density").text().as_double();
    Material *material = new Material(name, specific_heat, density,
                                      normal_conductivity, planar_conductivity);
    device->addMaterial(material);
  }

  /* Making the floorplan database */
  pugi::xml_node floorplan_nodes = doc.child("device").child("floorplans");
  for (pugi::xml_node floorplan_node = floorplan_nodes.child("floorplan");
       floorplan_node;
       floorplan_node = floorplan_node.next_sibling("floorplan")) {
    name = floorplan_node.attribute("name").value();
    Floorplan *floorplan = new Floorplan(name);
    for (pugi::xml_node tile_node = floorplan_node.child("tile"); tile_node;
         tile_node = tile_node.next_sibling("tile")) {
      name = tile_node.attribute("name").value();
      auto l = (VALUE)tile_node.child("length").text().as_double();
      auto w = (VALUE)tile_node.child("width").text().as_double();
      auto x = (VALUE)tile_node.child("x").text().as_double();
      auto y = (VALUE)tile_node.child("y").text().as_double();

      SubComponent *subComponents =
          new SubComponent(name, l, w, 0, x, y, 0, NULL);
      floorplan->addSubComponent(subComponents);
    }
    device->addFloorplan(floorplan);
  }
  /* Making the component list */
  pugi::xml_node component_nodes = doc.child("device").child("components");
  for (pugi::xml_node component_node = component_nodes.child("component");
       component_node;
       component_node = component_node.next_sibling("component")) {
    name = component_node.attribute("name").value();
    auto l = (VALUE)component_node.child("length").text().as_double();
    auto w = (VALUE)component_node.child("width").text().as_double();
    auto h = (VALUE)component_node.child("height").text().as_double();
    auto x = (VALUE)component_node.child("x").text().as_double();
    auto y = (VALUE)component_node.child("y").text().as_double();
    auto z = (VALUE)component_node.child("z").text().as_double();

    // Length, width, and height cannot be zero
    // Assertions to check it
    ASSERT(l != 0, "The length of " << name << " cannot be zero.");
    ASSERT(w != 0, "The width of " << name << " cannot be zero.");
    ASSERT(h != 0, "The height of " << name << " cannot be zero.");
    Component *component = new Component(name, l, w, h, x, y, z);

    string material = component_node.child("material").child_value();
    if (material.empty()) {
      component->setMaterial(NULL);
    } else {
      component->setMaterial(device->getMaterial(material));
    }

    string floorplan = component_node.child("floorplan").child_value();
    if (floorplan.empty()) {
      component->setFloorplan(NULL);
    } else {
      component->setFloorplan(device->getFloorplan(floorplan));
    }
    string power_gen = component_node.child("power").attribute("gen").value();
    if (power_gen.compare("yes") == 0) {
      component->setPowerGen(true);
      // may have gap filler
      string fill = component_node.child("power").child("fill").child_value();
      if (fill.compare("yes") == 0) {
        component->setFill(true);
        string fill_type = component_node.child("power")
                               .child("filling_material")
                               .child_value();
        component->setFillType(device->getMaterial(fill_type));
      } else {
        component->setFill(false);
      }
    } else {
      component->setPowerGen(false);
      // A device without power generation does not need any filling material
      component->setFill(false);
    }

    string lateral_connectivity =
        component_node.child("lateral_connectivity").child_value();
    if (lateral_connectivity.compare("yes") == 0) {
      component->setLateralConnectiviy(true);
    } else {
      component->setLateralConnectiviy(false);
    }

    /* updating the resolution value for the component */
    int width =
        component_node.child("resolution").child("width").text().as_int();
    int length =
        component_node.child("resolution").child("length").text().as_int();
    int height =
        component_node.child("resolution").child("height").text().as_int();
    if (width != 0 && length != 0 && height != 0)
      component->setResolution(length, width, height);

    device->addCompopnent(component);
  }
  /* sanity check */
  device->componentsOverlap();

  /* Fill the gaps with filling materials */
  device->fillGaps();

  /* sanity check to make sure that fillGaps() worked well */
  device->componentsOverlap();

  /* making subcomponents index considering the resolution of all the components
   */
  device->makeComponentsIndex();

  cout << design << " is parsed successfully.\n";
  return device;
}
