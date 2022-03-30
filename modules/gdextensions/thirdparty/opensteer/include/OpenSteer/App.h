// ----------------------------------------------------------------------------
//
//
// OpenSteer -- Steering Behaviors for Autonomous Characters
//
// Copyright (c) 2002-2003, Sony Computer Entertainment America
// Original author: Craig Reynolds <craig_reynolds@playstation.sony.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//
// ----------------------------------------------------------------------------
//
//
// OpenSteer App
//
// This class encapsulates the state of the OpenSteerDemo application and
// the services it provides to its plug-ins
//
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 11-14-02 cwr: recast App class as OpenSteerDemo 
// 06-26-02 cwr: App class created 
//
//
// ----------------------------------------------------------------------------


#ifndef OPENSTEER_APP_H
#define OPENSTEER_APP_H


#include "OpenSteer/Vec3.h"
#include "OpenSteer/Clock.h"
#include "OpenSteer/PlugIn.h"
#include "OpenSteer/Camera.h"
#include "OpenSteer/Utilities.h"

#include <sstream>

namespace OpenSteer {


    class App
    {
    public:
        // ------------------------------------------------------ component objects

        // clock keeps track of both "real time" and "simulation time"
        Clock clock;
        // camera automatically tracks selected vehicle
        Camera camera;

        // ------------------------------------------ addresses of selected objects

        // currently selected plug-in (user can choose or cycle through them)
        PlugIn* selectedPlugIn;

        // currently selected vehicle.  Generally the one the camera follows and
        // for which additional information may be displayed.  Clicking the mouse
        // near a vehicle causes it to become the Selected Vehicle.
        AbstractVehicle* selectedVehicle;

        // -------------------------------------------- initialize, update and exit

        App();
        ~App();

        // get App singleton object
        static App* get_singleton() {return singleton;}

        // main update function: step simulation forward and redraw scene
        void updateSimulationAndRedraw (void);

        // exit OpenSteerDemo with a given text message or error code
        void errorExit (const char* message);
        void exit (int exitCode);

        // ------------------------------------------------------- PlugIn interface

        // select the default PlugIn
        void selectDefaultPlugIn (void);
        // select the "next" plug-in, cycling through "plug-in selection order"
        void selectNextPlugIn (void);
        // handle function keys an a per-plug-in basis
        void functionKeyForPlugIn (int keyNumber);
        // return name of currently selected plug-in
        const char* nameOfSelectedPlugIn (void);
        // open the currently selected plug-in
        void openSelectedPlugIn (void);
        // do a simulation update for the currently selected plug-in
        void updateSelectedPlugIn (const float currentTime,
                                          const float elapsedTime);
        // redraw graphics for the currently selected plug-in
        void redrawSelectedPlugIn (const float currentTime,
                                          const float elapsedTime);
        // close the currently selected plug-in
        void closeSelectedPlugIn (void);
        // reset the currently selected plug-in
        void resetSelectedPlugIn (void);
        const AVGroup& allVehiclesOfSelectedPlugIn(void);

        // ---------------------------------------------------- OpenSteerDemo phase

        bool phaseIsDraw     (void) {return phase == drawPhase;}
        bool phaseIsUpdate   (void) {return phase == updatePhase;}
        bool phaseIsOverhead (void) {return phase == overheadPhase;}

        float phaseTimerDraw     (void) {return phaseTimers[drawPhase];}
        float phaseTimerUpdate   (void) {return phaseTimers[updatePhase];}
        // XXX get around shortcomings in current implementation, see note
        // XXX in updateSimulationAndRedraw
        //float phaseTimerOverhead(void){return phaseTimers[overheadPhase];}
        float phaseTimerOverhead (void)
        {
            return (clock.getElapsedRealTime() -
                    (phaseTimerDraw() + phaseTimerUpdate()));
        }

        // ------------------------------------------------------ delayed reset XXX

        // XXX to be reconsidered
        void queueDelayedResetPlugInXXX (void);
        void doDelayedResetPlugInXXX (void);

        // ------------------------------------------------------ vehicle selection

        // select the "next" vehicle: cycle through the registry
        void selectNextVehicle (void);
        // select vehicle nearest the given screen position (e.g.: of the mouse)
        void selectVehicleNearestScreenPosition (int x, int y);

        // ---------------------------------------------------------- mouse support

        // Find the AbstractVehicle whose screen position is nearest the
        // current the mouse position.  Returns NULL if mouse is outside
        // this window or if there are no AbstractVehicles.
        AbstractVehicle* vehicleNearestToMouse (void);

        // Find the AbstractVehicle whose screen position is nearest the
        // given window coordinates, typically the mouse position.  Note
        // this will return NULL if there are no AbstractVehicles.
        AbstractVehicle* findVehicleNearestScreenPosition (int x, int y);

        // for storing most recent mouse state
        int mouseX;
        int mouseY;
        bool mouseInWindow;

        // ------------------------------------------------------- camera utilities

        // set a certain initial camera state used by several plug-ins
        void init2dCamera (AbstractVehicle& selected);
        void init2dCamera (AbstractVehicle& selected,
                           float distance,
                           float elevation);
        void init3dCamera (AbstractVehicle& selected);
        void init3dCamera (AbstractVehicle& selected,
                           float distance,
                           float elevation);

        // set initial position of camera based on a vehicle
        void position3dCamera (AbstractVehicle& selected);
        void position3dCamera (AbstractVehicle& selected,
                               float distance,
                               float elevation);
        void position2dCamera (AbstractVehicle& selected);
        void position2dCamera (AbstractVehicle& selected,
                               float distance,
                               float elevation);

        // camera updating utility used by several (all?) plug-ins
        void updateCamera (const float currentTime,
                           const float elapsedTime,
                           const AbstractVehicle& selected);

        // return a normalized direction vector pointing from the camera towards a
        // given point on the screen: the ray that would be traced for that pixel
        Vec3 cameraToScreenPosition (int x, int y);

        // some camera-related default constants
        const float camera2dElevation = 8;
        const float cameraTargetDistance = 13;
        const Vec3 cameraTargetOffset = Vec3  (0, OpenSteer::App::camera2dElevation, 0);

        // ------------------------------------------------ graphics and annotation

        // get drawing view size
        int drawViewWidth();
        int drawViewHeight();

        // ground plane grid-drawing utility used by several plug-ins
        void gridUtility (const Vec3& gridTarget, bool withLines = false);
        // draws a gray disk on the XZ plane under a given vehicle
        void highlightVehicleUtility (const AbstractVehicle& vehicle);
        // draws a gray circle on the XZ plane under a given vehicle
        void circleHighlightVehicleUtility (const AbstractVehicle& vehicle);
        // draw a box around a vehicle aligned with its local space
        // xxx not used as of 11-20-02
        void drawBoxHighlightOnVehicle (const AbstractVehicle& v,
                                               const Vec3 color);
        // draws a colored circle (perpendicular to view axis) around the center
        // of a given vehicle.  The circle's radius is the vehicle's radius times
        // radiusMultiplier.
        void drawCircleHighlightOnVehicle (const AbstractVehicle& v,
                                                  const Vec3 color,
                                                  const float radiusMultiplier = 1);

        // graphical annotation: master on/off switch
        bool annotationIsOn (void) {return enableAnnotation;}
        void setAnnotationOn (void) {enableAnnotation = true;}
        void setAnnotationOff (void) {enableAnnotation = false;}
        bool toggleAnnotationState (void) {return (enableAnnotation = !enableAnnotation);}

        // ----------------------------------------------------------- console text

        // print a line on the console with "OpenSteerDemo: " then the given ending
        void printMessage (const char* message);
        void printMessage (const std::ostringstream& message);

        // like printMessage but prefix is "OpenSteerDemo: Warning: "
        void printWarning (const char* message);
        void printWarning (const std::ostringstream& message);

        // print list of known commands
        void keyboardMiniHelp (void);

        // ---------------------------------------------------------------- private

    private:
        bool enableAnnotation;

        // manage App phase transitions (xxx and maintain phase timers)

        static const int drawPhase = 2;
        static const int updatePhase = 1;
        static const int overheadPhase = 0;
        static const int phaseStackSize = 5;

        int phase;
        int phaseStack[phaseStackSize];
        int phaseStackIndex;
        float phaseTimers[drawPhase+1];
        float phaseTimerBase;
        void pushPhase (const int newPhase);
        void popPhase (void);
        void initPhaseTimers (void);
        void updatePhaseTimers (void);

        static App *singleton;
    };

    // Minimum set of drawing/graphics routines used in annotations and plugins
    namespace Draw {
        // Define scene's camera (viewing transformation) in terms of the camera's
        // position, the point to look at (an "aim point" in the scene which will
        // end up at the center of the camera's view), and an "up" vector defining
        // the camera's "roll" around the "view axis" between cameraPosition and
        // pointToLookAt (the image of the up vector will be vertical in the
        // camera's view).
        void drawCameraLookAt (const Vec3& cameraPosition, const Vec3& pointToLookAt, const Vec3& up);
        // draw 3d "graphical annotation" lines, used for debugging
        void drawLine (const Vec3& startPoint,
                       const Vec3& endPoint,
                       const Vec3& color);
        // draw a line with alpha blending
        void drawLine (const Vec3& startPoint,
                       const Vec3& endPoint,
                       const Vec3& color,
                       const float alpha);
        // draw a wide line
        void drawWideLine (const Vec3& startPoint,
                           const Vec3& endPoint,
                           const Vec3& color,
                           const float width);
        // draw lines grid around center point with given color
        void drawLineGrid (int hseg, int vseg,
                           const Vec3& center,
                           const Vec3& color);
        // General purpose circle/disk drawing routine.  Draws circles or disks (as
        // specified by "filled" argument) and handles both special case 2d circles
        // on the XZ plane or arbitrary circles in 3d space (as specified by "in3d"
        // argument)
        void drawCircle (const float radius,
                         const Vec3& axis,
                         const Vec3& center,
                         const Vec3& color,
                         const int segments,
                         const bool filled = false,
                         const bool in3d = false);
        // General purpose 2d circle/disk drawing routine.  Draws circles or disks
        // on the horizontal (XZ) plane
        void drawCircle (const float radius,
                         const Vec3& center,
                         const Vec3& color,
                         const int segments,
                         const bool filled = false);
        // General purpose quadrangle drawing routine. Draw quadrangle from given 4 corners
        // and color.
        void drawQuadrangle (const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& p4,
                             const Vec3& color);
        // Draw a (filled-in, polygon-based) square checkerboard grid on the horizontal
        // (XZ) plane.
        // ("size" is the length of a side of the overall grid, "subsquares" is the
        // number of subsquares along each edge (for example a standard checkboard
        // has eight), "center" is the 3d position of the center of the grid,
        // color1 and color2 are used for alternating subsquares.)
        void drawCheckerboardGrid (const float size,
                                   const int subsquares,
                                   const Vec3& center,
                                   const Vec3& color1,
                                   const Vec3& color2);
        // Draw the edges of a box with a given position, orientation, size
        // and color. The box edges are aligned with the axes of the given
        // LocalSpace, and it is centered at the origin of that LocalSpace.
        // "size" is the main diagonal of the box.
        void drawBox  (const AbstractLocalSpace& localSpace,
                       const Vec3& size,
                       const Vec3& color,
                       bool filled = false);
        // Draw circle with given color and radius. It is centered
        // at the origin of the given LocalSpace.
        void drawCircle (const AbstractLocalSpace& localSpace,
                         const Vec3& color,
                         float radius,
                         bool filled = false,
                         float up_offset = 0);
        // Draw text at 2d or 3d position
        void drawTextAt2dLocation(const std::ostringstream& text, const Vec3& position, const Vec3& color);
        void drawTextAt2dLocation(const char *text, const Vec3& position, const Vec3& color);
        void drawTextAt3dLocation(const std::ostringstream& text, const Vec3& position, const Vec3& color);
        void drawTextAt3dLocation(const char *text, const Vec3& position, const Vec3& color);
    } // namespace Draw

} // namespace OpenSteer


// ----------------------------------------------------------------------------

#endif // OPENSTEER_APP_H
