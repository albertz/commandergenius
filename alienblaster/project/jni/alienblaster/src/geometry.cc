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
 * goemetry.cc
 *
 * implements:
 *   class Vector2D
 *   namespace Geometry
 *   class Line
 *   class Circle
 *   class Rectangle
 *
 * requires:
 *
 * Maintainer: wenns
 *
 * Changelog:
 *
 *************************************************************************************/

//Furthermore it contains some goniometric functions to work with sine, cosine
//and tangent functions using degrees and some utility functions to return
//the maximum and the minimum of two values.

#include "geometry.h"
#include <algorithm>  // max and min
#include <stdio.h>     // needed for sprintf

AngDeg Rad2Deg ( AngRad x           ) { return ( x * 180 / M_PI ); }
AngRad Deg2Rad ( AngDeg x           ) { return ( x * M_PI / 180 ); }
float cosDeg   ( AngDeg x           ) { return ( cos( Deg2Rad( x ) ) ); }
float sinDeg   ( AngDeg x           ) { return ( sin( Deg2Rad( x ) ) ); }
float tanDeg   ( AngDeg x           ) { return ( tan( Deg2Rad( x ) ) ); }
AngDeg atanDeg ( float x            ) { return ( Rad2Deg( atan( x ) ) ); }

float atan2Deg( float x, float y ){
  if( fabs( x ) < EPSILON && fabs( y ) < EPSILON )
    return ( 0.0 );

  return ( Rad2Deg( atan2( x, y ) ) );
}


AngDeg acosDeg( float x ){
  if( x >= 1 )
    return ( 0.0 );
  else if( x <= -1 )
    return ( 180.0 );

  return ( Rad2Deg( acos( x ) ) );
}


AngDeg asinDeg( float x ){
  if( x >= 1 )
    return ( 90.0 );
  else if ( x <= -1 )
    return ( -90.0 );

  return ( Rad2Deg( asin( x ) ) );
}


float normalizeTo180Deg(float angle){
  while( angle > 180.0  ) angle -= 360.0;
  while( angle < -180.0 ) angle += 360.0;

  return ( angle );
}


bool isAngInInterval( AngDeg ang, AngDeg angMin, AngDeg angMax ){
  // convert all angles to interval 0..360
  if( ( ang    + 360 ) < 360 ) ang    += 360;
  if( ( angMin + 360 ) < 360 ) angMin += 360;
  if( ( angMax + 360 ) < 360 ) angMax += 360;

  if( angMin < angMax ) // 0 ---false-- angMin ---true-----angMax---false--360
    return angMin < ang && ang < angMax ;
  else                  // 0 ---true--- angMax ---false----angMin---true---360
    return !( angMax < ang && ang < angMin );
}


AngDeg getBisectorTwoAngles( AngDeg angMin, AngDeg angMax ){
  // separate sine and cosine part to circumvent boundary problem
  //return Vector2D::normalizeAngle(
  return normalizeTo180Deg(   //wenns changed
                    atan2Deg( (sinDeg( angMin) + sinDeg( angMax ) )/2.0,
                              (cosDeg( angMin) + cosDeg( angMax ) )/2.0 ) );
}

AngDeg getAngleDifference( AngDeg ang1, AngDeg ang2 ){
  return (normalizeTo180Deg( ang1 - ang2 ));
}

AngDeg getAbsAngleDifference( AngDeg ang1, AngDeg ang2 ){
  return fabs(normalizeTo180Deg( ang1 - ang2 ));
}

/******************************************************************************/
/********************   CLASS VECTOR2D   **************************************/
/******************************************************************************/

Vector2D::Vector2D( float x, float y, CoordSystemT cs ){
  setVector2D( x, y, cs );
}


ostream& operator <<( ostream &os, Vector2D v ){
  return ( os << "( " << v.x << ", " << v.y << " )" );
}


void Vector2D::show( CoordSystemT cs ){
  if( cs == CARTESIAN )
    cout << *this << endl;
  else
    cout << "( r: " << getLength( ) << ", phi: " << getDirection( ) << "  )";
}

/*
bool Vector2D::isInvalid() const {
  return (isUndefined(x)  || (isUndefined(y))
        //  || (fabs(x) >= 100000.0) || (fabs(y) >= 100000.0)
          || isinf(x) || isinf(y)  || isnan(x) || isnan(y));
}
*/

void Vector2D::setVector2D( float dX, float dY, CoordSystemT cs){
  if( cs == CARTESIAN )  { x = dX; y = dY; }
  else                   *this = getVector2DFromPolar( dX, dY );
}



Vector2D Vector2D::setLength( float d ){
  if( getLength( ) > EPSILON )
    ( *this ) *= ( d / getLength( ) );

  return ( *this );
}

Vector2D Vector2D::rotate( AngDeg angle ){
  // determine the polar representation of the current Vector2D
  float dMag    = this->getLength( );
  float dNewDir = this->getDirection( ) + angle;  // add rotation angle to phi
  setVector2D( dMag, dNewDir, POLAR );          // convert back to Cartesian
  return ( *this );
}

Vector2D Vector2D::globalToRelative( Vector2D origin, AngDeg ang ){
  // convert global coordinates into relative coordinates by aligning relative
  // frame and world frame. First perform translation to make origins of both
  // frames coincide. Then perform rotation to make axes of both frames coincide
  // (use negative angle since you rotate relative frame to world frame).
  *this -= origin;
  return ( rotate( -ang ) );
}

Vector2D Vector2D::relativeToGlobal( Vector2D origin, AngDeg ang ){
  // convert relative coordinates into global coordinates by aligning world
  // frame and relative frame. First perform rotation to make axes of both
  // frames coincide (use positive angle since you rotate world frame to
  // relative frame). Then perform translation to make origins of both frames
  // coincide.
  rotate( ang );
  *this += origin;
  return ( *this );
}

Vector2D Vector2D::getVector2DOnLineFraction( Vector2D &p,
                                                       float      dFrac ){
  // determine point on line that lies at fraction dFrac of whole line
  // example: this --- 0.25 ---------  p
  // formula: this + dFrac * ( p - this ) = this - dFrac * this + dFrac * p =
  //          ( 1 - dFrac ) * this + dFrac * p
  return ( ( *this ) * ( 1.0 - dFrac ) + ( p * dFrac ) );
}


Vector2D Vector2D::getVector2DFromPolar( float dMag, AngDeg ang ){
  // cos(phi) = x/r <=> x = r*cos(phi); sin(phi) = y/r <=> y = r*sin(phi)
  return ( Vector2D( dMag * cosDeg( ang ), dMag * sinDeg( ang ) ) );
}


/******************************************************************************/
/*********************** NAMESPACE GEOMETRY ***************************************/
/******************************************************************************/

float Geometry::getLengthGeomSeries( float dFirst, float dRatio, float dSum ){
  if( dRatio < 0 )
    cerr << "(Geometry:getLengthGeomSeries): negative ratio" << endl;

  // s = a + ar + ar^2 + .. + ar^n-1 and thus sr = ar + ar^2 + .. + ar^n
  // subtract: sr - s = - a + ar^n) =>  s(1-r)/a + 1 = r^n = temp
  // log r^n / n = n log r / log r = n = length
  float temp = (dSum * ( dRatio - 1 ) / dFirst) + 1;
  if( temp <= 0 )
    return -1.0;
  return log( temp ) / log( dRatio ) ;
}


float Geometry::getSumGeomSeries( float dFirst, float dRatio, float dLength){
  // s = a + ar + ar^2 + .. + ar^n-1 and thus sr = ar + ar^2 + .. + ar^n
  // subtract: s - sr = a - ar^n) =>  s = a(1-r^n)/(1-r)
  return dFirst * ( 1 - pow( dRatio, dLength ) ) / ( 1 - dRatio ) ;
}


float Geometry::getSumInfGeomSeries( float dFirst, float dRatio ){
  if( dRatio > 1 )
    cerr << "(Geometry:CalcLengthGeomSeries): series does not converge" << endl;

  // s = a(1-r^n)/(1-r) with n->inf and 0<r<1 => r^n = 0
  return dFirst / ( 1 - dRatio );
}


float Geometry::getFirstGeomSeries( float dSum, float dRatio, float dLength){
  // s = a + ar + ar^2 + .. + ar^n-1 and thus sr = ar + ar^2 + .. + ar^n
  // subtract: s - sr = a - ar^n) =>  s = a(1-r^n)/(1-r) => a = s*(1-r)/(1-r^n)
  return dSum *  ( 1 - dRatio )/( 1 - pow( dRatio, dLength ) ) ;
}

float Geometry::getFirstInfGeomSeries( float dSum, float dRatio ){
  if( dRatio > 1 )
    cerr << "(Geometry:getFirstInfGeomSeries):series does not converge" << endl;

  // s = a(1-r^n)/(1-r) with r->inf and 0<r<1 => r^n = 0 => a = s ( 1 - r)
  return dSum * ( 1 - dRatio );
}

int Geometry::abcFormula(float a, float b, float c, float *s1, float *s2){
  float dDiscr = b*b - 4*a*c;       // discriminant is b^2 - 4*a*c
  if (fabs(dDiscr) < EPSILON )       // if discriminant = 0
  {
    *s1 = -b / (2 * a);              //  only one solution
    return 1;
  }
  else if (dDiscr < 0)               // if discriminant < 0
    return 0;                        //  no solutions
  else                               // if discriminant > 0
  {
    dDiscr = sqrt(dDiscr);           //  two solutions
    *s1 = (-b + dDiscr ) / (2 * a);
    *s2 = (-b - dDiscr ) / (2 * a);
    return 2;
  }
}

/******************************************************************************/
/********************** CLASS CIRCLE ******************************************/
/******************************************************************************/

Circle::Circle( Vector2D pos, float dR ){
  setCircle( pos, dR );
}

Circle::Circle( ){
  setCircle( Vector2D(-1000.0,-1000.0), 0);
}

void Circle::show( ostream& os) const {
  os << "c:" << posCenter << ", r:" << radius;
}

bool Circle::setCircle( const Vector2D &pos, float dR ){
  setCenter( pos );
  return setRadius( dR  );
}
bool Circle::setRadius( float dR ){
  if( dR > 0 ){
    radius = dR;
    return true;
  }
  else{
    radius = 0.0;
    return false;
  }
}


int Circle::getIntersectionPoints( const Circle &c, Vector2D *p1,
				   Vector2D *p2) const {
    float x0, y0, r0;
    float x1, y1, r1;

    x0 = getCenter( ).getX();
    y0 = getCenter( ).getY();
    r0 = getRadius( );
    x1 = c.getCenter( ).getX();
    y1 = c.getCenter( ).getY();
    r1 = c.getRadius( );

    float      d, dx, dy, h, a, x, y, p2_x, p2_y;

    // first calculate distance between two centers circles P0 and P1.
    dx = x1 - x0;
    dy = y1 - y0;
    d = sqrt(dx*dx + dy*dy);

    // normalize differences
    dx /= d; dy /= d;

    // a is distance between p0 and point that is the intersection point P2
    // that intersects P0-P1 and the line that crosses the two intersection
    // points P3 and P4.
    // Define two triangles: P0,P2,P3 and P1,P2,P3.
    // with distances a, h, r0 and b, h, r1 with d = a + b
    // We know a^2 + h^2 = r0^2 and b^2 + h^2 = r1^2 which then gives
    // a^2 + r1^2 - b^2 = r0^2 with d = a + b ==> a^2 + r1^2 - (d-a)^2 = r0^2
    // ==> r0^2 + d^2 - r1^2 / 2*d
    a = (r0*r0 + d*d - r1*r1) / (2.0 * d);

    // h is then a^2 + h^2 = r0^2 ==> h = sqrt( r0^2 - a^2 )
    float      arg = r0*r0 - a*a;
    h = (arg > 0.0) ? sqrt(arg) : 0.0;

    // First calculate P2
    p2_x = x0 + a * dx;
    p2_y = y0 + a * dy;

    // and finally the two intersection points
    x =  p2_x - h * dy;
    y =  p2_y + h * dx;
    p1->setVector2D( x, y );
    x =  p2_x + h * dy;
    y =  p2_y - h * dx;
    p2->setVector2D( x, y );

    return (arg < 0.0) ? 0 : ((arg == 0.0 ) ? 1 :  2);
}


float Circle::getIntersectionArea( const Circle &c ) const {
  Vector2D pos1, pos2, pos3;
  float d, h, dArea;
  AngDeg ang;

  d = getCenter().distanceTo( c.getCenter() ); // dist between two centers
  if( d > c.getRadius() + getRadius() )           // larger than sum radii
    return 0.0;                                   // circles do not intersect
  if( d <= fabs(c.getRadius() - getRadius() ) )   // one totally in the other
  {
    float dR = min( c.getRadius(), getRadius() );// return area smallest circle
    return M_PI*dR*dR;
  }

  int iNrSol = getIntersectionPoints( c, &pos1, &pos2 );
  if( iNrSol != 2 )
    return 0.0;

  // the intersection area of two circles can be divided into two segments:
  // left and right of the line between the two intersection points p1 and p2.
  // The outside area of each segment can be calculated by taking the part
  // of the circle pie excluding the triangle from the center to the
  // two intersection points.
  // The pie equals pi*r^2 * rad(2*ang) / 2*pi = 0.5*rad(2*ang)*r^2 with ang
  // the angle between the center c of the circle and one of the two
  // intersection points. Thus the angle between c and p1 and c and p3 where
  // p3 is the point that lies halfway between p1 and p2.
  // This can be calculated using ang = asin( d / r ) with d the distance
  // between p1 and p3 and r the radius of the circle.
  // The area of the triangle is 2*0.5*h*d.

  pos3 = pos1.getVector2DOnLineFraction( pos2, 0.5 );
  d = pos1.distanceTo( pos3 );
  h = pos3.distanceTo( getCenter() );
  ang = asin( d / getRadius() );

  dArea = ang*getRadius()*getRadius();
  dArea = dArea - d*h;

  // and now for the other segment the same story
  h = pos3.distanceTo( c.getCenter() );
  ang = asin( d / c.getRadius() );
  dArea = dArea + ang*c.getRadius()*c.getRadius();
  dArea = dArea - d*h;

  return dArea;
}


bool Circle::calcTangentIntersectionPoints(const Vector2D startPoint, Vector2D &point1, Vector2D &point2){
  if(isInside(startPoint)){
    // Startpoint is inside circle -> there are no tangent interception points
    return(false);
  }

  //float d = posCenter.getLength()-startPoint.getLength();
  float d = (posCenter-startPoint).getLength();
  float r = radius;

  float alphaRad = asin(r/d);

  float p = sqrt(d*d-r*r);

  point1.setX(cos(alphaRad)*p);
  point1.setY(sin(alphaRad)*p);
  point2.setX(cos(-alphaRad)*p);
  point2.setY(sin(-alphaRad)*p);
  
  point1=point1.rotate((posCenter-startPoint).getDirection());
  point2=point2.rotate((posCenter-startPoint).getDirection());

  point1+=startPoint;
  point2+=startPoint;

  return(true);
}






/******************************************************************************/
/***********************  CLASS LINE *******************************************/
/******************************************************************************/

Line::Line( float dA, float dB, float dC ){
  a = dA;
  b = dB;
  c = dC;
}

Line::Line() {
  a = 666.66;
  b = 666.66;
  c = 666.66;
}

ostream& operator <<(ostream & os, Line l){
  float a = l.getACoefficient();
  float b = l.getBCoefficient();
  float c = l.getCCoefficient();

  // ay + bx + c = 0 -> y = -b/a x - c/a
  if( a == 0 )
    os << "x = " << -c/b;
  else{
    os << "y = ";
    if( b != 0 )
      os << -b/a << "x ";
    if( c > 0 )
       os << "- " <<  fabs(c/a);
    else if( c < 0 )
       os << "+ " <<  fabs(c/a);
  }
  return os;
}

void Line::show( ostream& os){
  os << *this;
}

Vector2D Line::getIntersection( Line line ){
  Vector2D pos(666.66,666.66);
  float x, y;

  if( (b == line.getBCoefficient())&&(b!=-a) )  { // lines are parallel, no intersection
    return pos;
  }
  if( a == 0 ){               // bx + c = 0 and a2*y + b2*x + c2 = 0 ==> x = -c/b
    x = -c/b;                 // calculate x using the current line
    y = line.getYGivenX(x);   // and calculate the y using the second line
  }

  // ay + bx + c = 0 and b2*x + c2 = 0 ==> x = -c2/b2
  // calculate x using 2nd line and calculate y using current line
  else if( line.getACoefficient() == 0 ){
   x = -line.getCCoefficient()/line.getBCoefficient();
   y = getYGivenX(x);
  }

  // ay + bx + c = 0 and a2y + b2*x + c2 = 0
  // y = (-b2/a2)x - c2/a2
  // bx = -a*y - c =>  bx = -a*(-b2/a2)x -a*(-c2/a2) - c ==>
  // ==> a2*bx = a*b2*x + a*c2 - a2*c ==> x = (a*c2 - a2*c)/(a2*b - a*b2)
  // calculate x using the above formula and the y using the current line
  else{
    x = (a*line.getCCoefficient() - line.getACoefficient()*c)/
                    (line.getACoefficient()*b - a*line.getBCoefficient());
    y = getYGivenX(x);
  }

  return Vector2D( x, y );
}


int Line::getCircleIntersectionPoints( Circle circle,
              Vector2D *posSolution1, Vector2D *posSolution2 ){
  int    iSol;
  float dSol1, dSol2;
  float h = circle.getCenter().getX();
  float k = circle.getCenter().getY();

  // line:   x = -c/b (if a = 0)
  // circle: (x-h)^2 + (y-k)^2 = r^2, with h = center.x and k = center.y
  // fill in:(-c/b-h)^2 + y^2 -2ky + k^2 - r^2 = 0
  //         y^2 -2ky + (-c/b-h)^2 + k^2 - r^2 = 0
  // and determine solutions for y using abc-formula
  if( fabs(a) < EPSILON ){
    iSol = Geometry::abcFormula( 1, -2*k, ((-c/b) - h)*((-c/b) - h)
              + k*k - circle.getRadius()*circle.getRadius(), &dSol1, &dSol2);
    posSolution1->setVector2D( (-c/b), dSol1 );
    posSolution2->setVector2D( (-c/b), dSol2 );
    return iSol;
  }

  // ay + bx + c = 0 => y = -b/a x - c/a, with da = -b/a and db = -c/a
  // circle: (x-h)^2 + (y-k)^2 = r^2, with h = center.x and k = center.y
  // fill in:x^2 -2hx + h^2 + (da*x-db)^2 -2k(da*x-db) + k^2 - r^2 = 0
  //         x^2 -2hx + h^2 + da^2*x^2 + 2da*db*x + db^2 -2k*da*x -2k*db
  //                                                         + k^2 - r^2 = 0
  //         (1+da^2)*x^2 + 2(da*db-h-k*da)*x + h2 + db^2  -2k*db + k^2 - r^2 = 0
  // and determine solutions for x using abc-formula
  // fill in x in original line equation to get y coordinate
  float da = -b/a;
  float db = -c/a;

  float dA = 1 + da*da;
  float dB = 2*( da*db - h - k*da );
  float dC = h*h + db*db - 2*k*db + k*k - circle.getRadius()*circle.getRadius();

  iSol = Geometry::abcFormula( dA, dB, dC, &dSol1, &dSol2 );

  posSolution1->setVector2D( dSol1, da*dSol1 + db );
  posSolution2->setVector2D( dSol2, da*dSol2 + db );
  return iSol;
}


Line Line::getTangentLine( Vector2D pos ){ // Senkrechte errichten, nix mit Tangente
  // ay + bx + c = 0 -> y = (-b/a)x + (-c/a)
  // tangent: y = (a/b)*x + C1 -> by - ax + C2 = 0 => C2 = ax - by
  // with pos.y = y, pos.x = x
  return Line( b, -a, a*pos.getX() - b*pos.getY() );
}


Vector2D Line::getPointOnLineClosestTo( Vector2D pos ){
  Line l2 = getTangentLine( pos );  // get tangent line
  return getIntersection( l2 );     // and intersection between the two lines
}


float Line::distanceToPoint( Vector2D pos ){
  return pos.distanceTo( getPointOnLineClosestTo( pos ) );
}


bool Line::isInBetween( Vector2D pos, Vector2D point1, Vector2D point2){
  pos          = getPointOnLineClosestTo( pos ); // get closest point
  float dDist = point1.distanceTo( point2 ); // get distance between 2 pos
  // if the distance from both points to the projection is smaller than this
  // dist, the pos lies in between.
  return pos.distanceTo( point1 ) <= dDist &&
         pos.distanceTo( point2 ) <= dDist;
}


float Line::getYGivenX( float x ){
 if( a == 0 ){
   cerr << "(Line::getYGivenX) Cannot calculate Y coordinate: " ;
   show( cerr );
   cerr << endl;
   return 0;
 }
  // ay + bx + c = 0 ==> ay = -(b*x + c)/a
  return -(b*x+c)/a;
}


float Line::getXGivenY( float y ){
 if( b == 0 ){
   cerr << "(Line::getXGivenY) Cannot calculate X coordinate\n" ;
   return 0;
 }
  // ay + bx + c = 0 ==> bx = -(a*y + c)/a
  return -(a*y+c)/b;
}


Line Line::makeLineFromTwoPoints( Vector2D pos1, Vector2D pos2 ){
  // 1*y + bx + c = 0 => y = -bx - c
  // with -b the direction coefficient (or slope)
  // and c = - y - bx
  float dA=1.0, dB, dC;
  float dTemp = pos2.getX() - pos1.getX(); // determine the slope
  if( fabs(dTemp) < EPSILON ){
    // ay + bx + c = 0 with vertical slope=> a = 0, b = 1
    dA = 0.0;
    dB = 1.0;
  }
  else{
    // y = (-b)x -c with -b the slope of the line
    dA = 1.0;
    dB = -(pos2.getY() - pos1.getY())/dTemp;
  }
  // ay + bx + c = 0 ==> c = -a*y - b*x
  dC =  - dA*pos2.getY()  - dB * pos2.getX();
  return Line( dA, dB, dC );
}


Line Line::makeLineFromPositionAndAngle( Vector2D vec, AngDeg angle ){
  // calculate point somewhat further in direction 'angle' and make
  // line from these two points.
  return makeLineFromTwoPoints( vec, vec+Vector2D(1,angle,POLAR));
}

/******************************************************************************/
/********************** CLASS RECTANGLE ***************************************/
/******************************************************************************/

RectangleGeo::RectangleGeo( Vector2D pos, Vector2D pos2 ){
  setRectanglePoints( pos, pos2 );
}


void RectangleGeo::setRectanglePoints( Vector2D pos1, Vector2D pos2 ){
  posLeftTop.setX    ( min( pos1.getX(), pos2.getX() ) );
  posLeftTop.setY    ( min( pos1.getY(), pos2.getY() ) );
  posRightBottom.setX( max( pos1.getX(), pos2.getX() ) );
  posRightBottom.setY( max( pos1.getY(), pos2.getY() ) );
}


void RectangleGeo::show( ostream& os ) const {
  os << "rect(" << posLeftTop << " " << posRightBottom << ")";
}


bool RectangleGeo::isInside( Vector2D pos ) const {
  return ( (getPosLeftTop().getX() <= pos.getX()) &&
	   (pos.getX() <= getPosRightBottom().getX()) &&
	   (getPosLeftTop().getY() <= pos.getY()) &&
	   (pos.getY() <= getPosRightBottom().getY()) );
}


/******************************************************************************/
/********************** CLASS CONE ********************************************/
/******************************************************************************/

Cone::Cone( Vector2D peak,
	    float fstAngle, float sndAngle,
	    float maxDistance, float minDistance ) {
  setPeak( peak );
  setAngles( fstAngle, sndAngle );
  setMaxDistance( maxDistance );
  setMinDistance( minDistance );
}

void Cone::show( ostream& os ) const {
  os << "(p:" << peak << " fst:" << fstAngle << " snd:" << sndAngle 
     <<  " maxDist:" << maxDistance << " minDist:" << minDistance << ")";
}

bool Cone::isInside( Vector2D pos ) const {
  float dist = pos.distanceTo( peak );
  float angle = (pos - peak).getDirection();
  return ( ( minDistance <= dist && dist <= maxDistance &&
	     isAngInInterval( angle, fstAngle, sndAngle ) ) ||
	   ( dist == 0 && minDistance == 0) );
}





/******************************************************************************/
/********************** TESTING PURPOSES *************************************/
/******************************************************************************/

/*
#include<iostream.h>

int main( void ){
  float dFirst = 1.0;
  float dRatio = 2.5;
  float dSum   = 63.4375;
  float dLength = 4.0;

  printf( "sum: %f\n", Geometry::getSumGeomSeries( dFirst, dRatio, dLength));
  printf( "length: %f\n", Geometry::getLengthGeomSeries( dFirst, dRatio, dSum));
}

int main( void ){
  Line l1(1,-1,3 );
  Line l2(1,-0.2,10 );
  Line l3 = Line::makeLineFromTwoPoints( Vector2D(1,-1), Vector2D(2,-2) );
  l3.show();
  cout << endl;
  l1.show();
  l2.show();
  l1.getIntersection( l2 ).show();
}

int main( void ){
  Line l( 1, -1, 0 );
  Vector2D s1, s2;
  int i = l.getCircleIntersectionPoints( Circle( Vector2D(1,1), 1 ), &s1, &s2 );
  printf( "number of solutions: %d\n", i );
  if( i == 2 ){
    cout << s1 << " " << s2 ;
  }
  else if( i == 1 ){
    cout << s1;
  }
  cout << "line: " << l << endl;
}

int main( void ){
  Circle c11( Vector2D( 10, 0 ), 10);
  Circle c12( Vector2D( 40, 3 ), 40 );
  Circle c21( Vector2D(  0,0 ), 5);
  Circle c22( Vector2D(  3,0 ), 40 );

  Vector2D p1, p2;

  cout << c11.getIntersectionArea( c21 ) << endl;
  cout << c12.getIntersectionArea( c21 ) << endl;
  cout << c22.getIntersectionArea( c11 ) << endl;
  cout << c12.getIntersectionArea( c22 ) << endl;
  return 0;
}

int main( void ){
  cout << getBisectorTwoAngles( -155.3, 179.0 ) << endl;
  cout << getBisectorTwoAngles( -179.3, 179.0 ) << endl;
}

int main( ) {
  Vector2D pos = Vector2D::getVector2DFromPolar(10, 45);
  cout << pos << endl;
  cout << pos.getDirection() << endl;
  cout << pos.getLength() << endl;

  //Vector2D pos = Vector2D(10, 0);
  //cout << pos << endl;
  //cout << pos.getDirection() << endl;
  //pos.rotate(90);
  //cout << pos << endl;
  //cout << pos.getDirection() << endl;
}

int main( ) {
  Cone c( Vector2D(-10,-10), -170, -150, 50 );
  c.show();
  cout << endl 
       << "isInside( Vector2D(0,0)): " << c.isInside( Vector2D(0,0) ) << endl
       << "isInside( Vector2D(0,-5)): " << c.isInside( Vector2D(0,-5) ) << endl
       << "isInside( Vector2D(-12,-11)): " << c.isInside( Vector2D(-12,-11) ) << endl;
}
*/
 




