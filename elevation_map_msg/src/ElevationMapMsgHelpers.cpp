/*
 * ElevationMapMsgHelpers.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: Péter Fankhauser
 *	 Institute: ETH Zurich, Autonomous Systems Lab
 */

#include "elevation_map_msg/ElevationMapMsgHelpers.hpp"

// Elevation Map
#include "elevation_map_msg/TransformationMath.hpp""

// ROS
#include <ros/ros.h>

// Boost
#include <boost/assign.hpp>

namespace elevation_map_msg {

const int nDimensions()
{
  return 2;
}

std::map<StorageIndices, std::string> storageIndexNames = boost::assign::map_list_of
    (StorageIndices::Column,  "column_index")
    (StorageIndices::Row, "row_index");

bool isRowMajor(const std_msgs::Float32MultiArray& messageData)
{
  if (messageData.layout.dim[0].label == storageIndexNames[StorageIndices::Column]) return false;
  else if (messageData.layout.dim[0].label == storageIndexNames[StorageIndices::Row]) return true;

  ROS_ERROR("elevation_map_msg: isRowMajor() failed because layout label is not set correctly.");
  return false;
}

unsigned int getCols(const std_msgs::Float32MultiArray& messageData)
{
  if (isRowMajor(messageData)) return messageData.layout.dim.at(1).size;
  return messageData.layout.dim.at(0).size;
}

unsigned int getRows(const std_msgs::Float32MultiArray& messageData)
{
  if (isRowMajor(messageData)) return messageData.layout.dim.at(0).size;
  return messageData.layout.dim.at(1).size;
}

bool isValidCell(const float elevation, const float variance)
{
  return !(std::isnan(elevation) || std::isinf(variance));
}

unsigned int get1dIndexFrom2dIndex(
    const Eigen::Array2i& index,
    const elevation_map_msg::ElevationMap& map)
{
  unsigned int n;

  if(!isRowMajor(map.elevation)) n = map.elevation.layout.data_offset + index(1) * map.elevation.layout.dim[1].stride + index(0);
  else n = map.elevation.layout.data_offset + index(0) * map.elevation.layout.dim[1].stride + index(1);

  return n;
}

//Eigen::Array2i get2dIndexFrom1dIndex(
//    unsigned int n, const elevation_map_msg::ElevationMap& map)
//{
//  Eigen::Vector2i index;
//  index(1) = n - map.elevation.layout.data_offset % map.elevation.layout.dim[1].stride;
//  index(0) = (int)((n - map.elevation.layout.data_offset - index(1)) / map.elevation.layout.dim[1].stride);
//  return index;
//}

bool getPositionFromIndex(Eigen::Vector2d& position,
                          const Eigen::Array2i& index,
                          const elevation_map_msg::ElevationMap& map)
{
  Array2d mapLength(map.lengthInX, map.lengthInY);
  Vector2d mapPosition(map.position.x, map.position.y);
  Array2i bufferSize = (getBufferOrderToMapFrameAlignment() * Vector2i(getRows(map.elevation), getCols(map.elevation))).array();
  Array2i bufferStartIndex = (getBufferOrderToMapFrameAlignment() * Vector2i(map.outerStartIndex, map.innerStartIndex)).array();
  return getPositionFromIndex(position, index, mapLength, mapPosition, map.resolution, bufferSize, bufferStartIndex);
}

bool getIndexFromPosition(Eigen::Array2i& index,
                          const Eigen::Vector2d& position,
                          const elevation_map_msg::ElevationMap& map)
{
  Array2d mapLength(map.lengthInX, map.lengthInY);
  Vector2d mapPosition(map.position.x, map.position.y);
  Array2i bufferSize = (getBufferOrderToMapFrameAlignment() * Vector2i(getRows(map.elevation), getCols(map.elevation))).array();
  Array2i bufferStartIndex = (getBufferOrderToMapFrameAlignment() * Vector2i(map.outerStartIndex, map.innerStartIndex)).array();
  return getIndexFromPosition(index, position, mapLength, mapPosition, map.resolution, bufferSize, bufferStartIndex);
}

bool copyColorValueToVector(const unsigned long& colorValue,
                            Eigen::Vector3i& colorVector)
{
  colorVector(0) = (colorValue >> 16) & 0x0000ff;
  colorVector(1) = (colorValue >> 8) & 0x0000ff;
  colorVector(2) =  colorValue & 0x0000ff;
  return true;
}

bool copyColorValueToVector(const unsigned long& colorValue,
                            Eigen::Vector3f& colorVector)
{
  Vector3i tempColorVector;
  copyColorValueToVector(colorValue, tempColorVector);
  colorVector = ((tempColorVector.cast<float>()).array() / 255.0).matrix();
  return true;
}

bool copyColorVectorToValue(const Eigen::Vector3i& colorVector,
                            unsigned long& colorValue)
{
  colorValue = ((int)colorVector(0)) << 16 | ((int)colorVector(1)) << 8 | ((int)colorVector(2));
  return true;
}

} // namespace