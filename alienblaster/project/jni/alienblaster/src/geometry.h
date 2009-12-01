/*
Copyright (c) 2000,2001, Jelle Kok, University of Amsterdam
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the University of Amsterdam nor the names of its
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**************************************************************************************
 * geometry.hh
 *
 * implements:
 *   class Vector2D
 *   namespace Geometry
 *   class Line
 *   class Circle
 *   class Rectangle
 *   class Cone
 *
 * requires:
 *
 * Maintainer: wenns, pagra
 *
 * Changelog:
 *
 * Note:
 *************************************************************************************/

#ifndef GEOMETRY_HH
#define GEOMETRY_HH

#include <math.h>         // needed for M_PI constant
#include <iostream>
using namespace std;

const float EPSILON  = 0.001;

typedef float AngRad;  /**< Type definition for angles in degrees. */
typedef float AngDeg;  /**< Type definition for angles in radians. */

/**
 * Converts an angle in radians to the corresponding angle in degrees.
 * @param x an angle in radians
 * @return the corresponding angle in degrees
 */
AngDeg Rad2Deg ( AngRad x           );

/**
 * Converts an angle in degrees to the corresponding angle in radians.
 * @param x an angle in degrees
 * @return the corresponding angle in radians
 */
AngRad Deg2Rad ( AngDeg x           );

/**
 * Returns the cosine of a given angle in degrees using the
 * built-in cosine function that works with angles in radians.
 * @param x an angle in degrees
 * @return the cosine of the given angle
 */
float cosDeg   ( AngDeg x           );

/**
 * Returns the sine of a given angle in degrees using the
 * built-in sine function that works with angles in radians.
 * @param x an angle in degrees
 * @return the sine of the given angle
 */
float sinDeg   ( AngDeg x           );

/**
 * Returns the tangent of a given angle in degrees using the
 * built-in tangent function that works with angles in radians.
 * @param x an angle in degrees
 * @return the tangent of the given angle
 */
float tanDeg   ( AngDeg x           );

/**
 * Rreturns the principal value of the arc tangent of x in degrees
 * using the built-in arc tangent function which returns this value in radians.
 * @param x a float value
 * @return the arc tangent of the given value in degrees
 */
AngDeg atanDeg ( float x            );

/**
 * Returns the principal value of the arc tangent of y/x in
 * degrees using the signs of both arguments to determine the quadrant of the
 * return value. For this the built-in 'atan2' function is used which returns
 * this value in radians.
 * @param x a float value
 * @param y a float value
 * @return the arc tangent of y/x in degrees taking the signs of x and y into
 * account
 */
float atan2Deg ( float x,  float y  );

/**
 * Returns the principal value of the arc cosine of x in degrees
 * using the built-in arc cosine function which returns this value in radians.
 * @param x a float value
 * @return the arc cosine of the given value in degrees
 */
AngDeg acosDeg ( float x            );

/**
 * Returns the principal value of the arc sine of x in degrees
 * using the built-in arc sine function which returns this value in radians.
 * @param x a float value
 * @return the arc sine of the given value in degrees
 */
AngDeg asinDeg ( float x            );

/**
 * Returns a boolean value which indicates whether the value
 * 'ang' (from interval [-180..180] lies in the interval [angMin..angMax].
 * Examples: isAngInInterval( -100, 4, -150) returns false
 *           isAngInInterval(   45, 4, -150) returns true
 * @param ang angle that should be checked
 * @param angMin minimum angle in interval
 * @param angMax maximum angle in interval
 * @return boolean indicating whether ang lies in [angMin..angMax]
 */
bool   isAngInInterval     ( AngDeg ang,    AngDeg angMin, AngDeg angMax );

/**
 * Returns the bisector (average) of two angles. It deals
 * with the boundary problem, thus when 'angMin' equals 170 and 'angMax'
 * equals -100, -145 is returned.
 * @param angMin minimum angle [-180,180]
 * @param angMax maximum angle [-180,180]
 * @return average of angMin and angMax.
 */
AngDeg getBisectorTwoAngles( AngDeg angMin, AngDeg angMax );

/**
 * Returns the difference of two angles in degrees [-180,180]
 * @param ang1 first  angle [-180,180]
 * @param ang2 second angle [-180,180]
 */
AngDeg getAngleDifference( AngDeg ang1, AngDeg ang2 );


/**
 * Returns the absolute difference of two angles in degrees [0,180]
 * @param ang1 first  angle [-180,180]
 * @param ang2 second angle [-180,180]
 */
AngDeg getAbsAngleDifference( AngDeg ang1, AngDeg ang2 );


/**
 * Normalizes an arbitrary angle in 180 Deg System
 * @param angle: the angle to normalize
 * @return float (-180..180]
 */
float normalizeTo180Deg(float angle);


/**
 * CoordSystem is an enumeration of the different specified coordinate systems.
 * The two possibilities are CARTESIAN or POLAR. These values are for instance
 * used in the initializing a Vector2D. The CoordSystem indicates whether
 * the supplied arguments represent the position in cartesian or in polar
 * coordinates.
 */
enum CoordSystemT {
  CARTESIAN,
  POLAR
};

/******************************************************************************/
/********************   CLASS VECTOR2D   **************************************/
/******************************************************************************/

/**
 * This class contains an x- and y-coordinate of a position (x,y) as member
 * data and methods which operate on this position. The standard arithmetic
 * operators are overloaded and can thus be applied to positions (x,y). It is
 * also possible to represent a position in polar coordinates (r,phi), since
 * the class contains a method to convert these into cartesian coordinates (x,y).
 */
class Vector2D {
  private:
    float x;   /**< x-coordinate of this position */
    float y;   /**< y-coordinate of this position */

  public:

    /**
    * Constructor for the Vector2D class. When the supplied Coordinate System
    * type equals CARTESIAN, the arguments x and y denote the x- and y-coordinates
    * of the new position. When it equals POLAR however, the arguments x and y
    * denote the polar coordinates of the new position; in this case x is thus
    * equal to the distance r from the origin and y is equal to the angle phi that
    * the polar vector makes with the x-axis.
    * @param x the x-coordinate of the new position when cs == CARTESIAN; the
    * distance of the new position from the origin when cs = POLAR
    * @param y the y-coordinate of the new position when cs = CARTESIAN; the
    * angle that the polar vector makes with the x-axis when cs = POLAR
    * @param cs a CoordSystemT indicating whether x and y denote cartesian
    * coordinates or polar coordinates
    * @return the Vector2D corresponding to the given arguments
    */
    Vector2D                               ( float            x = 0,
                                             float            y = 0,
                                             CoordSystemT     cs = CARTESIAN);

    //////////////////// overloaded arithmetic operators  /////////////////////////////
    /*
     * Note: all operators parametrized with float values apply these values to both
     * coordinates of the vector
     */

    /**
     * Overloaded version of unary minus operator for Vector2Ds. The current Vector2D
     * itself is left unchanged.
     * @return a negated version of the current Vector2D
     */
    Vector2D operator -  (                   ) { return ( Vector2D( -x, -y ) );           }

    Vector2D operator +  ( const float    &d ) const { return ( Vector2D( x + d, y + d ) );     }
    Vector2D operator +  ( const Vector2D &p ) const { return ( Vector2D( x + p.x, y + p.y ) ); }
    Vector2D operator -  ( const float    &d ) const { return ( Vector2D( x - d, y - d ) );     }
    Vector2D operator -  ( const Vector2D &p ) const { return ( Vector2D( x - p.x, y - p.y ) ); }
    Vector2D operator *  ( const float    &d ) const { return ( Vector2D( x * d, y * d  ) );    }
    Vector2D operator *  ( const Vector2D &p ) const { return ( Vector2D( x * p.x, y * p.y ) ); }
    Vector2D operator /  ( const float    &d ) const { return ( Vector2D( x / d, y / d  ) );    }
    Vector2D operator /  ( const Vector2D &p ) const { return ( Vector2D( x / p.x, y / p.y ) ); }
    void     operator =  ( const float    &d ) { x = d; y = d;                            }
    void     operator += ( const Vector2D &p ) { x += p.x; y += p.y;                      }
    void     operator += ( const float    &d ) { x += d; y += d;                          }
    void     operator -= ( const Vector2D &p ) { x -= p.x; y -= p.y;                      }
    void     operator -= ( const float    &d ) { x -= d; y -= d;                          }
    void     operator *= ( const Vector2D &p ) { x *= p.x; y *= p.y;                      }
    void     operator *= ( const float    &d ) { x *= d; y *= d;                          }
    void     operator /= ( const Vector2D &p ) { x /= p.x; y /= p.y;                      }
    void     operator /= ( const float    &d ) { x /= d; y /= d;                          }
    bool     operator != ( const Vector2D &p ) const { return ( ( x != p.x ) || ( y != p.y ) ); }
    bool     operator != ( const float    &d ) const { return ( ( x != d ) || ( y != d ) );     }
    bool     operator == ( const Vector2D &p ) const { return ( ( x == p.x ) && ( y == p.y ) ); }
    bool     operator == ( const float    &d ) const { return ( ( x == d ) && ( y == d ) );     }

    //////////////////////// methods for producing output ////////////////////////////
    friend ostream& operator <<       ( ostream           &os,
	                                     Vector2D                p );
    /**
     * Writes the current Vector2D to standard output. It can also print a polar
     * representation of the current Vector2D.
     * @param cs a CoordSystemtT indicating whether a POLAR or CARTESIAN
     * representation of the current Vector2D should be printed
     */
    void               show                ( CoordSystemT      cs = CARTESIAN );

    // accessors
    void     setX         ( float newX )       { x = newX; }
    float    getX         (            ) const { return x; }
    void     setY         ( float newY )       { y = newY; }
    float    getY         (            ) const { return y; }

    /**
     * decides if the Vector2D is invalid
     * @return true, if the Vector2D is invalid, false otherwise
     */
    bool isInvalid() const;

    /**
     * (re)sets the coordinates of the current Vector2D. The given
     * coordinates can either be polar or Cartesian coordinates.
     * @param newX a float value indicating either a new Cartesian x-coordinate when
     *        cs=CARTESIAN or a new polar r-coordinate (distance) when cs=POLAR
     * @param newY a float value indicating either a new Cartesian y-coordinate when
     *        cs=CARTESIAN or a new polar phi-coordinate (angle) when cs=POLAR
     * @param cs a CoordSystemT indicating whether x and y denote cartesian
     *        coordinates or polar coordinates
     */
    void     setVector2D  ( float        newX = 0,
                            float        newY = 0,
                            CoordSystemT cs = CARTESIAN);

    //wenns: temp workaround
    //void     setVector2D_Orig( float dX, float dY, CoordSystemT cs);

    /**
     * Determines the distance between the current Vector2D and a
     * given Vector2D.
     * @param p a Vector2D
     * @return the distance between the current Vector2D and the given
     * Vector2D
     */
    inline float    distanceTo   ( const Vector2D &p ) const
    { return ( ( *this - p ).getLength( ) ); }

    /**
     * Adjusts the coordinates of the current Vector2D in such a way
     * that the length of the corresponding vector equals the float value which
     * is supplied as an argument.
     * @param f a float value representing a new length
     * @return the result of scaling the vector corresponding with the current
     * Vector2D to the given length thus yielding a different Vector2D
     */
    Vector2D setLength    ( float f          );

    /**
     * This method determines the length of Vector2D using the formula of Pythagoras.
     * @return the length of the Vector2D
     */
    inline float    getLength    (                  ) const { return ( sqrt( x * x + y * y ) ); }

    /**
     * This method determines the direction of the current Vector2D (the phi-coordinate
     * in polar representation)
     * @return the direction in degrees of the current Vector2D
     */
    inline AngDeg   getDirection (                  ) const { return ( atan2Deg( y, x ) );      }

    /////////////////////// comparison methods for positions ///////////////////////////

    /**
     * Determines whether the current Vector2D is in front of a
     * given Vector2D, i.e. whether the x-coordinate of the current Vector2D
     * is larger than the x-coordinate of the given Vector2D.
     * @param p a Vector2D to which the current Vector2D must be compared
     * @return true when the current Vector2D is in front of the given
     * Vector2D; false otherwise
     */
    bool     isInFrontOf  ( const Vector2D &p  )
    { return ( ( x > p.getX( ) ) ? true : false ); }
    /**< similar as above */
    bool     isInFrontOf  ( const float    &d  ) { return ( ( x > d ) ? true : false ); }

    /**
     * Determines whether the current Vector2D is behind a given
     * Vector2D, i.e. whether the x-coordinate of the current Vector2D is
     * smaller than the x-coordinate of the given Vector2D.
     * @param p a Vector2D to which the current Vector2D must be compared
     * @return true when the current Vector2D is behind the given Vector2D;
     * false otherwise
     */
    bool     isBehindOf   ( const Vector2D &p  )
    { return ( ( x < p.getX( ) ) ? true : false ); }
    /**< similar as above */
    bool     isBehindOf   ( const float    &d  ) { return ( ( x < d ) ? true : false ); }

    /**
     * Determines whether the current Vector2D is to the left of a
     * given Vector2D, i.e. whether the y-coordinate of the current Vector2D
     * is smaller than the y-coordinate of the given Vector2D.
     * @param p a Vector2D to which the current Vector2D must be compared
     * @return true when the current Vector2D is to the left of the given
     * Vector2D; false otherwise
     */
    bool     isLeftOf     ( const Vector2D &p  )
    { return ( ( y < p.getY( ) ) ? true : false ); }
    /**< similar as above */
    bool     isLeftOf     ( const float    &d  ) { return ( ( y < d ) ? true : false ); }

    /**
     * This method determines whether the current Vector2D is to the right of a
     * given Vector2D, i.e. whether the y-coordinate of the current Vector2D
     * is larger than the y-coordinate of the given Vector2D.
     * @param p a Vector2D to which the current Vector2D must be compared
     * @return true when the current Vector2D is to the right of the given
     * Vector2D; false otherwise
     */
    bool     isRightOf    ( const Vector2D &p  )
    { return ( ( y > p.getY( ) ) ? true : false ); }
    /**< similar as above */
    bool     isRightOf    ( const float    &d  ) { return ( ( y > d ) ? true : false ); }

    /**
     * Determines whether the current Vector2D is in between two
     * given Vector2Ds when looking in the x-direction, i.e. whether the current
     * Vector2D is in front of the first argument and behind the second.
     * @param p1 a Vector2D to which the current Vector2D must be compared
     * @param p2 a Vector2D to which the current Vector2D must be compared
     * @return true when the current Vector2D is in between the two given
     * Vector2Ds when looking in the x-direction; false otherwise
     */
    inline bool     isBetweenX   ( const Vector2D &p1,
                            const Vector2D &p2 )
    { return ( ( isInFrontOf( p1 ) && isBehindOf( p2 ) ) ? true : false ); }
    /**< similar as above */
    inline bool     isBetweenX   ( const float    &d1,
                            const float    &d2 )
    { return ( ( isInFrontOf( d1 ) && isBehindOf( d2 ) ) ? true : false ); }

    /**
     * Determines whether the current Vector2D is in between two
     * given Vector2Ds when looking in the y-direction, i.e. whether the current
     * Vector2D is to the right of the first argument and to the left of the
     * second.
     * @param p1 a Vector2D to which the current Vector2D must be compared
     * @param p2 a Vector2D to which the current Vector2D must be compared
     * @return true when the current Vector2D is in between the two given
     * Vector2Ds when looking in the y-direction; false otherwise
     */
    bool     isBetweenY   ( const Vector2D &p1,
                            const Vector2D &p2 )
    { return ( ( isRightOf( p1 ) && isLeftOf( p2 ) ) ? true : false ); }
    /**< similar as above */
    bool     isBetweenY   ( const float    &d1,
                            const float    &d2 )
    { return ( ( isRightOf( d1 ) && isLeftOf( d2 ) ) ? true : false ); }

    /////////////////// conversion methods for positions ////////////////////////////

    /**
     * Normalizes a Vector2D by setting the length of the
     * corresponding vector to 1.
     * @return the result of normalizing the current Vector2D
     */
    inline Vector2D normalize                (                ) { return ( setLength( 1.0 ) ); }

    /**
     * Rotates the current Vector2D over a given angle .
     * @param angle an angle in degrees over which the current Vector2D must be rotated
     * @return the result of rotating the current Vector2D over the given angle
     */
    Vector2D rotate                   ( AngDeg   angle );

    /**
     * Converts the coordinates of the current Vector2D (which are represented in an
     * global coordinate system with the origin at (0,0)) into relative coordinates
     * in a different coordinate system (e.g. relative to a player). The new coordinate
     * system is defined by the arguments to the method.
     * @param origin the origin of the relative coordinate frame
     * @param ang the angle between the world frame and the relative frame
     * (reasoning from the world frame)
     * @return the result of converting the current global Vector2D into a
     * relative Vector2D
     */
    Vector2D globalToRelative         ( Vector2D orig,
                                        AngDeg   ang   );
    /**< similar to above */
    Vector2D relativeToGlobal         ( Vector2D orig,
                                        AngDeg   ang   );
    /**
     * Returns a Vector2D that lies somewhere on the vector between
     * the current Vector2D and a given Vector2D. The desired position is
     * specified by a given fraction of this vector (e.g. 0.5 means exactly in
     * the middle of the vector).
     * @param p a Vector2D which defines the vector to the current Vector2D
     * @param dFrac float representing the fraction of the connecting vector at
     * which the desired Vector2D lies.
     * @return the Vector2D which lies at fraction dFrac on the vector
     * connecting p and the current Vector2D
     */
    Vector2D getVector2DOnLineFraction( Vector2D &p,
                                        float    dFrac );

    /**
     * Converts a polar representation of a Vector2D into a Cartesian.
     * @param dMag a float representing the polar r-coordinate, i.e. the distance
     * from the point to the origin
     * @param ang the angle that the polar vector makes with the x-axis, i.e. the
     * polar phi-coordinate
     * @return the result of converting the given polar representation into a
     * Cartesian representation thus yielding a Cartesian Vector2D
     */
    //static Vector2D getVector2DFromPolar( float            dMag,
    //                                      AngDeg            ang           );

    //wenns: temp workaround
    static Vector2D getVector2DFromPolar/*_Orig*/( float dMag, AngDeg ang );
};

/******************************************************************************/
/*********************   NAMESPACE GEOMETRY   *********************************/
/******************************************************************************/

namespace Geometry {
  ///////////////////////////// geometric series //////////////////////////////////

  /*
   * A geometric series is one in which there is a constant ratio between each
   * element and the one preceding it.
   * Normally: s = a + ar + ar^2 + ...  + ar^n
   * Now: Sum = First + First*Ratio + First*Ratio^2 + .. + Fist*Ratio^n
   */

  /**
   * Determines the length of a geometric series given its first element, the sum
   * of the elements in the series and the constant ratio between the elements.
   * @param first first term of the series
   * @param ratio ratio with which the the first term is multiplied
   * @param sum the total sum of all the serie
   * @return the length(n in above example) of the series
   */
  float getLengthGeomSeries (float first, float ratio, float sum      );

  /**
   * Determines the sum of a geometric series given its first element, the ratio and
   * the number of steps in the series
   * @param first first term of the series
   * @param ratio ratio with which the the first term is multiplied
   * @param length the number of steps to be taken into account
   * @return the sum of the series
   */
  float getSumGeomSeries     ( float first, float ratio, float length );
  /**
   * Determines the sum of an infinite geometric series given its first element and
   * the constant ratio between the elements. Note that such an infinite series will
   * only converge when 0<r<1.
   * @param first first term of the series
   * @param ratio ratio with which the the first term is multiplied
   * @return the sum of the series
   */
  float getSumInfGeomSeries  ( float first, float ratio               );

  /**
   * Determines the first element of a geometric series given its element, the ratio
   * and the number of steps in the series
   * @param sum sum of the series
   * @param ratio ratio with which the the first term is multiplied
   * @param length the number of steps to be taken into account
   * @return the first element (a) of a serie
   */
  float getFirstGeomSeries   ( float sum,   float ratio, float length );

  /**
   * Determines the first element of an infinite geometric series given its first
   * element and the constant ratio between the elements. Note that such an infinite
   * series will only converge when 0<r<1.
   * @param sum sum of the series
   * @param ratio ratio with which the the first term is multiplied
   * @return the first term of the series
   */
  float getFirstInfGeomSeries ( float sum, float ratio                );

  /////////////////////////////// Pythagoras' Theorem ////////////////////////////////
  /**
   * Performs the abc formula (Pythagoras' Theorem) on the given parameters and puts
   * the result in *s1 en *s2. It returns the number of found coordinates.
   * @param a a parameter in abc formula
   * @param b b parameter in abc formula
   * @param c c parameter in abc formula
   * @param *s1 first result of abc formula
   * @param *s2 second result of abc formula
   * @return number of found x-coordinates
   */
  int   abcFormula ( float a, float b, float c, float *s1, float *s2  );
};

/******************************************************************************/
/********************** CLASS CIRCLE ******************************************/
/******************************************************************************/

/**
 * This class represents a circle. A circle is defined by one Vector2D
 * (which denotes the center) and its radius.
 */
class Circle{
  Vector2D posCenter;            /**< Center of the circle  */
  float    radius;               /**< Radius of the circle  */
  
public:
  Circle( );
  Circle( Vector2D pos, float r );
  
  void show ( ostream& os = cout ) const;
  
  // accessors
  /**
   * Sets the values of the circle.
   * @param pos new center of the circle
   * @param r new radius of the circle ( > 0 )
   * @return bool indicating whether radius was set
   */
  bool setCircle ( const Vector2D &pos, float r );
  
  /**
   * Sets the radius of the circle.
   * @param r new radius of the circle ( > 0 )
   * @return bool indicating whether radius was set
   */
  bool setRadius ( const float r );
  void setCenter ( const Vector2D &pos ) { posCenter = pos; }
  
  float getRadius ()        const { return radius; }
  Vector2D getCenter ()     const { return posCenter; }
  float getCircumference () const { return 2.0*M_PI*getRadius(); }
  float getArea ()          const { return M_PI*getRadius()*getRadius(); }

  /**
   * Returns a boolean that indicates whether 'pos' is located inside the circle.
   * @param pos position of which should be checked whether it is located in the
   * circle
   * @return bool indicating whether pos lies inside the circle
   */
  bool isInside ( const Vector2D &pos ) const
  { return posCenter.distanceTo( pos ) < getRadius(); }
  
  /**
   * Returns the two possible intersection points between two circles. This method
   * returns the number of solutions that were found.
   * @param c circle with which intersection should be found
   * @param p1 will be filled with first solution
   * @param p2 will be filled with second solution
   * @return number of solutions.
   */
  int getIntersectionPoints ( const Circle &c, Vector2D *p1, Vector2D *p2) const;

  /**
   * Returns the size of the intersection area of two circles.
   * @param c circle with which intersection should be determined
   * @return size of the intersection area.
   */
  float getIntersectionArea ( const Circle &c ) const ;


  bool calcTangentIntersectionPoints(const Vector2D, Vector2D &point1, Vector2D &point2);

};


/******************************************************************************/
/*********************** CLASS LINE *******************************************/
/******************************************************************************/

/**
 * This class contains the representation of a line. A line is defined
 * by the formula ay + bx + c = 0. The coefficients a, b and c are stored
 * and used in the calculations.
 */
class Line {
  // a line is defined by the formula: ay + bx + c = 0
  float a; /**< This is the a coefficient in the line ay + bx + c = 0 */
  float b; /**< This is the b coefficient in the line ay + bx + c = 0 */
  float c; /**< This is the c coefficient in the line ay + bx + c = 0 */

public:
  Line( float a, float b, float c );
  Line();

  //accessors
  float getACoefficient ( ) const { return a; }
  float getBCoefficient ( ) const { return b; }
  float getCCoefficient ( ) const { return c; }


  // print methods
  void        show( ostream& os = cout );
  friend      ostream& operator << (ostream & os, Line l);

  /**
   * Returns the intersection point between the current Line and the specified line.
   * @param line line with which the intersection should be calculated.
   * @return Vector2D position that is the intersection point.
   */
  Vector2D getIntersection            ( Line        line                   );

  /**
   * This method calculates the intersection points between the current line
   * and the circle specified with as center 'posCenter' and radius 'radius'.
   * The number of solutions are returned and the corresponding points are put
   * in the third and fourth argument of the method
   * @param c circle with which intersection points should be found
   * @param posSolution1 first intersection (if any)
   * @param posSolution2 second intersection (if any)
   */
  int      getCircleIntersectionPoints( Circle      circle,
                                        Vector2D *posSolution1,
                                        Vector2D *posSolution2          );

  /**
   * Returns the tangent line to a Vector2D. This is the line between the specified
   * position and the closest point on the line to this position.
   * @param pos Vector2D point with which tangent line is calculated.
   * @return Line line tangent to this position
   */
  Line     getTangentLine          ( Vector2D pos                    );

  /**
   * Returns the closest point on a line to a given position.
   * @param pos point to which closest point should be determined
   * @return Vector2D closest point on line to 'pos'.
   */
  Vector2D getPointOnLineClosestTo    ( Vector2D pos                    );

  /**
   * Returns the distance between a specified position and the closest point on
   * the given line.
   * @param pos position to which distance should be calculated
   * @return float indicating the distance to the line.
   */
  float      distanceToPoint       ( Vector2D pos                    );

  /**
   * Determines whether the projection of a point on the current line lies between
   * two other points ('point1' and 'point2') that lie on the same line.
   * @param pos point of which projection is checked.
   * @param point1 first point on line
   * @param point2 second point on line
   * @return true when projection of 'pos' lies between 'point1' and 'point2'.
   */
  bool        isInBetween                ( Vector2D pos,
                                           Vector2D point1,
                                           Vector2D point2                 );

  /**
   * Calculates the y coordinate given the x coordinate
   * @param x coordinate
   * @return y coordinate on this line
   */
  float getYGivenX                      ( float      x );

  /**
   * Calculates the x coordinate given the y coordinate
   * @param y coordinate
   * @return x coordinate on this line
   */
  float getXGivenY                      ( float      y );

  ///////  static methods to make a line using an easier representation  ////////
  /**
   * Creates a line given two points.
   * @param pos1 first point
   * @param pos2 second point
   * @return line that passes through the two specified points.
   */
  static Line makeLineFromTwoPoints        ( Vector2D pos1,
                                             Vector2D pos2                   );

  /**
   * Creates a line given a position and an angle.
   * @param vec position through which the line passes
   * @param angle direction of the line.
   * @return line that goes through position 'vec' with angle 'angle'.
   */
  static Line makeLineFromPositionAndAngle ( Vector2D vec,
                                             AngDeg angle                       );
};

/******************************************************************************/
/********************** CLASS RECTANGLE ***************************************/
/******************************************************************************/

/**
 * This class represents a rectangle. A rectangle is defined by two Vector2Ds
 * the one at the upper left corner and the one at the right bottom.
 */
class RectangleGeo {
  Vector2D posLeftTop;     /**< top left position of the rectangle       */
  Vector2D posRightBottom; /**< bottom right position of the rectangle   */

public:
  /**
   * This is the constructor of a Rectangle. Two points will be given. The
   * order does not matter as long as two opposite points are given (left
   * top and right bottom or right top and left bottom).
   * @param pos first point that defines corner of rectangle
   * @param pos2 second point that defines other corner of rectangle
   * @return rectangle with 'pos' and 'pos2' as opposite corners.
   */
  RectangleGeo                  ( Vector2D pos, Vector2D pos2 );

  void        show           ( ostream& os = cout          ) const;

  /**
   * Determines whether the given position lies inside the current rectangle.
   * @param pos position which is checked whether it lies in rectangle
   * @return true when 'pos' lies in the rectangle, false otherwise
   */
  bool     isInside          ( Vector2D pos                ) const;

  /**
   * Sets the upper left and right bottom point of the current rectangle.
   * @param pos first point that defines corner of rectangle
   * @param pos2 second point that defines other corner of rectangle
   */
  void     setRectanglePoints( Vector2D pos1,
                               Vector2D pos2               );

  void     setPosLeftTop     ( Vector2D pos                ) { posLeftTop = pos;      }
  void     setPosRightBottom ( Vector2D pos                ) { posRightBottom = pos;  }
  Vector2D getPosLeftTop     () const { return posLeftTop; }
  Vector2D getPosRightBottom () const { return posRightBottom; }
};



/******************************************************************************/
/********************** CLASS CONE **** ***************************************/
/******************************************************************************/

/**
 * This class represents a cone. A cone is defined by one point,
 * two angles (-180 to 180), a maximum and a minimum distance.
 * The point gives the peak of the cone. A point that lies in the cone
 * is at least minimum distance and at most maximum distance away from 
 * the peak. Additionally the point must lie in the segment between
 * the first angle (the left edge) and the second angle (the other edge).
 * The area is defined by rotating the first edge (given by the first angle)
 * clockwise (mit dem Uhrzeigersinn) till the second angle is reached.
 */
class Cone {
  Vector2D peak;     /**< peak of the cone */
  float fstAngle;    /**< the left edge. Start of clockwise rotation. */
  float sndAngle;    /**< the right edge. End of clockwise rotation. */
  float maxDistance; /**< the max. distance a point on the cone can be away from the peak. */
  float minDistance; /**< the min. distance a point on the cone must be away from the peak. */

public:
  /**
   * This is the constructor of a Cone. The cone lies between fstAngle and sndAngle,
   * where sndAngle is interpreted as being clockwise rotated in relation
   * to fstAngle.
   * @param peak first point that defines top of the cone
   * @param fstAngle absolute angle of the first edge
   * @param sndAngle absolute angle of the second edge. 
   * @param maxDistance max. distance a point on the cone can be away from the peak
   * @param minDistance min. distance a point on the cone must be away from the peak
   */
  Cone( Vector2D peak,
	float fstAngle, float sndAngle,
	float maxDistance, float minDistance = 0 );

  void show( ostream& os = cout ) const;

  /**
   * Determines whether the given position lies inside the current cone.
   * @param pos position which is checked whether it lies in cone
   * @return true when 'pos' lies in the cone, false otherwise
   */
  bool isInside( Vector2D pos ) const;

  inline void setPeak( Vector2D newPeak ) { peak = newPeak; }
  inline void setAngles( float newFstAngle, float newSndAngle ) {
    fstAngle = newFstAngle, sndAngle = newSndAngle; 
  }
  inline void setFstAngle( float newFstAngle ) { fstAngle = newFstAngle; }
  inline void setSndAngle( float newSndAngle ) { sndAngle = newSndAngle; }
  inline void setMaxDistance( float newMaxDistance) { maxDistance = newMaxDistance; }
  inline void setMinDistance( float newMinDistance) { minDistance = newMinDistance; }

};

#endif //Geometry.hh

