#pragma once

#include "IModel.h"
#include <Eigen/Dense>
#include <string>

namespace Model
{

/**
 * @class IGeometryModel
 * @brief Interface for geometry model operations
 *
 * This interface extends IModel with geometry-specific operations like
 * file I/O, transformations, and geometric queries.
 */
class IGeometryModel: public virtual IModel
{
public:
    using Matrix4d = Eigen::Matrix4d;
    using Vector3d = Eigen::Vector3d;

    /**
     * @brief Load geometry from a file
     * @param filename Path to the file
     * @return True if loading succeeded, false otherwise
     */
    virtual bool loadFromFile(const std::string& filename) = 0;

    /**
     * @brief Save geometry to a file
     * @param filename Path to the file
     * @return True if saving succeeded, false otherwise
     */
    virtual bool saveToFile(const std::string& filename) = 0;

    /**
     * @brief Apply a transformation matrix to the geometry
     * @param matrix 4x4 transformation matrix
     */
    virtual void transform(const Matrix4d& matrix) = 0;

    /**
     * @brief Get the bounding box of the geometry
     * @param min Minimum point of the bounding box (output)
     * @param max Maximum point of the bounding box (output)
     * @return True if the bounding box was computed successfully, false otherwise
     */
    virtual bool getBoundingBox(Vector3d& min, Vector3d& max) const = 0;

    /**
     * @brief Get the volume of the geometry
     * @return The volume or 0.0 if computation failed
     */
    virtual double getVolume() const = 0;

    /**
     * @brief Get the surface area of the geometry
     * @return The surface area or 0.0 if computation failed
     */
    virtual double getSurfaceArea() const = 0;
};

}  // namespace Model