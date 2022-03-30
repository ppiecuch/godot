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
// App
//
// This class encapsulates the state of the App application and the
// services it provides to its plug-ins.  It is never instantiated, all of
// its members are static (belong to the class as a whole.)
//
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 11-14-02 cwr: created 
//
//
// ----------------------------------------------------------------------------



#include "OpenSteer/SteerLibrary.h"
#include "OpenSteer/App.h"

#include <algorithm>
#include <string>
#include <sstream>

namespace OpenSteer {
    void draw3dCircleOrDisk (const float radius, const Vec3& center, const Vec3& axis, const Vec3& color, const int segments, const bool filled)
    {
        // draw a circle-or-disk in the given local space
        Draw::drawCircle (radius, axis, center, color, segments, filled, true);
    }
    void draw3dCircle (const float radius, const Vec3& center, const Vec3& axis, const Vec3& color, const int segments)
    {
        draw3dCircleOrDisk (radius, center, axis, color, segments, false);
    }
    void drawXZCircleOrDisk (const float radius, const Vec3& center, const Vec3& color, const int segments, const bool filled)
    {
        // draw a circle-or-disk on the XZ plane
        Draw::drawCircle (radius, Vec3::zero, center, color, segments, filled, false);
    }
    void drawXZCircle (const float radius, const Vec3& center, const Vec3& color, const int segments)
    {
        drawXZCircleOrDisk (radius, center, color, segments, false);
    }
    void drawXZDisk (const float radius, const Vec3& center, const Vec3& color, const int segments)
    {
        drawXZCircleOrDisk (radius, center, color, segments, true);
    }
}


OpenSteer::App* OpenSteer::App::singleton = NULL;


// ----------------------------------------------------------------------------
// initialize App application

namespace {

    void printPlugIn (OpenSteer::PlugIn& pi) {std::cout << " " << pi << std::endl;} // XXX

} // anonymous namespace

OpenSteer::App::App (void)
{
    singleton = this;

    enableAnnotation = true;
    phaseTimerBase = 0;
    phaseStackIndex = 0;

    mouseX = 0;
    mouseY = 0;
    mouseInWindow = false;

    // select the default PlugIn
    selectDefaultPlugIn ();

    {
        // XXX this block is for debugging purposes,
        // XXX should it be replaced with something permanent?
        std::cout << std::endl << "Known plugins:" << std::endl;   // xxx?
        PlugIn::applyToAll (printPlugIn);                          // xxx?
        std::cout << std::endl;                                    // xxx?
        // identify default PlugIn
        if (!selectedPlugIn) errorExit ("no default PlugIn");
        std::cout << std::endl << "Default plugin:" << std::endl;  // xxx?
        std::cout << " " << *selectedPlugIn << std::endl;          // xxx?
        std::cout << std::endl;                                    // xxx?
    }
    // initialize the default PlugIn
    openSelectedPlugIn ();
}

OpenSteer::App::~App (void)
{
    singleton = NULL;
}

// ----------------------------------------------------------------------------
// main update function: step simulation forward and redraw scene


void 
OpenSteer::App::updateSimulationAndRedraw (void)
{
    // update global simulation clock
    clock.update ();

    //  start the phase timer (XXX to accurately measure "overhead" time this
    //  should be in displayFunc, or somehow account for time outside this
    //  routine)
    initPhaseTimers ();
    // run selected PlugIn (with simulation's current time and step size)
    updateSelectedPlugIn (clock.getTotalSimulationTime (),
                          clock.getElapsedSimulationTime ());
    // redraw selected PlugIn (based on real time)
    redrawSelectedPlugIn (clock.getTotalRealTime (),
                          clock.getElapsedRealTime ());
}


// ----------------------------------------------------------------------------
// exit App with a given text message or error code


void 
OpenSteer::App::errorExit (const char* message)
{
    printMessage (message);
#ifdef _MSC_VER
	MessageBox(0, message, "App Unfortunate Event", MB_ICONERROR);
#endif
    exit (-1);
}


void 
OpenSteer::App::exit (int exitCode)
{
    ::exit (exitCode);
}


// ----------------------------------------------------------------------------
// select the default PlugIn


void 
OpenSteer::App::selectDefaultPlugIn (void)
{
    PlugIn::sortBySelectionOrder ();
    selectedPlugIn = PlugIn::findDefault ();
}


// ----------------------------------------------------------------------------
// select the "next" plug-in, cycling through "plug-in selection order"


void 
OpenSteer::App::selectNextPlugIn (void)
{
    closeSelectedPlugIn ();
    selectedPlugIn = selectedPlugIn->next ();
    openSelectedPlugIn ();
}


// ----------------------------------------------------------------------------
// handle function keys an a per-plug-in basis


void 
OpenSteer::App::functionKeyForPlugIn (int keyNumber)
{
    selectedPlugIn->handleFunctionKeys (keyNumber);
}


// ----------------------------------------------------------------------------
// return name of currently selected plug-in


const char* 
OpenSteer::App::nameOfSelectedPlugIn (void)
{
    return (selectedPlugIn ? selectedPlugIn->name() : "no PlugIn");
}


// ----------------------------------------------------------------------------
// open the currently selected plug-in


void 
OpenSteer::App::openSelectedPlugIn (void)
{
    camera.reset ();
    selectedVehicle = NULL;
    selectedPlugIn->open ();
}


// ----------------------------------------------------------------------------
// do a simulation update for the currently selected plug-in


void 
OpenSteer::App::updateSelectedPlugIn (const float currentTime,
                                                const float elapsedTime)
{
    // switch to Update phase
    pushPhase (updatePhase);
    // service queued reset request, if any
    doDelayedResetPlugInXXX ();
    // if no vehicle is selżcted, and some exist, select the first one
    if (selectedVehicle == NULL)
    {
        const AVGroup& vehicles = allVehiclesOfSelectedPlugIn();
        if (vehicles.size() > 0) selectedVehicle = vehicles.front();
    }
    // invoke selected PlugIn's Update method
    selectedPlugIn->update (currentTime, elapsedTime);
    // return to previous phase
    popPhase ();
}


// ----------------------------------------------------------------------------
// redraw graphics for the currently selected plug-in


void 
OpenSteer::App::redrawSelectedPlugIn (const float currentTime, const float elapsedTime)
{
    // switch to Draw phase
    pushPhase (drawPhase);
    // invoke selected PlugIn's Draw method
    selectedPlugIn->redraw (currentTime, elapsedTime);
    // return to previous phase
    popPhase ();
}


// ----------------------------------------------------------------------------
// close the currently selected plug-in


void 
OpenSteer::App::closeSelectedPlugIn (void)
{
    selectedPlugIn->close ();
    selectedVehicle = NULL;
}


// ----------------------------------------------------------------------------
// reset the currently selected plug-in


void 
OpenSteer::App::resetSelectedPlugIn (void)
{
    selectedPlugIn->reset ();
}


namespace {

    // ----------------------------------------------------------------------------
    // XXX this is used by CaptureTheFlag
    // XXX it was moved here from main.cpp on 12-4-02
    // XXX I'm not sure if this is a useful feature or a bogus hack
    // XXX needs to be reconsidered.


    bool gDelayedResetPlugInXXX = false;

} // anonymous namespace
    
    
void 
OpenSteer::App::queueDelayedResetPlugInXXX (void)
{
    gDelayedResetPlugInXXX = true;
}


void 
OpenSteer::App::doDelayedResetPlugInXXX (void)
{
    if (gDelayedResetPlugInXXX)
    {
        resetSelectedPlugIn ();
        gDelayedResetPlugInXXX = false;
    }
}


// ----------------------------------------------------------------------------
// return a group (an STL vector of AbstractVehicle pointers) of all
// vehicles(/agents/characters) defined by the currently selected PlugIn


const OpenSteer::AVGroup& 
OpenSteer::App::allVehiclesOfSelectedPlugIn (void)
{
    return selectedPlugIn->allVehicles ();
}


// ----------------------------------------------------------------------------
// select the "next" vehicle: the one listed after the currently selected one
// in allVehiclesOfSelectedPlugIn


void 
OpenSteer::App::selectNextVehicle (void)
{
    if (selectedVehicle != NULL)
    {
        // get a container of all vehicles
        const AVGroup& all = allVehiclesOfSelectedPlugIn ();
        const AVIterator first = all.begin();
        const AVIterator last = all.end();
        // find selected vehicle in container
        const AVIterator s = std::find (first, last, selectedVehicle);
        // normally select the next vehicle in container
        selectedVehicle = *(s+1);
        // if we are at the end of the container, select the first vehicle
        if (s == last-1) selectedVehicle = *first;
        // if the search failed, use NULL
        if (s == last) selectedVehicle = NULL;
    }
}


// ----------------------------------------------------------------------------
// select vehicle nearest the given screen position (e.g.: of the mouse)


void 
OpenSteer::App::selectVehicleNearestScreenPosition (int x, int y)
{
    selectedVehicle = findVehicleNearestScreenPosition (x, y);
}


// ----------------------------------------------------------------------------
// Find the AbstractVehicle whose screen position is nearest the current the
// mouse position.  Returns NULL if mouse is outside this window or if
// there are no AbstractVehicle.


OpenSteer::AbstractVehicle* 
OpenSteer::App::vehicleNearestToMouse (void)
{
    return (mouseInWindow ? 
            findVehicleNearestScreenPosition (mouseX, mouseY) :
            NULL);
}


// ----------------------------------------------------------------------------
// Find the AbstractVehicle whose screen position is nearest the given window
// coordinates, typically the mouse position.  Returns NULL if there are no
// AbstractVehicles.
//
// This works by constructing a line in 3d space between the camera location
// and the "mouse point".  Then it measures the distance from that line to the
// centers of each AbstractVehicle.  It returns the AbstractVehicle whose
// distance is smallest.
//
// xxx Issues: Should the distanceFromLine test happen in "perspective space"
// xxx or in "screen space"?  Also: I think this would be happy to select a
// xxx vehicle BEHIND the camera location.


OpenSteer::AbstractVehicle* 
OpenSteer::App::findVehicleNearestScreenPosition (int x, int y)
{
    // find the direction from the camera position to the given pixel
    const Vec3 direction = App::get_singleton()->cameraToScreenPosition (x, y);

    // iterate over all vehicles to find the one whose center is nearest the
    // "eye-mouse" selection line
    float minDistance = FLT_MAX;       // smallest distance found so far
    AbstractVehicle* nearest = NULL;   // vehicle whose distance is smallest
    const AVGroup& vehicles = allVehiclesOfSelectedPlugIn();
    for (AVIterator i = vehicles.begin(); i != vehicles.end(); i++)
    {
        // distance from this vehicle's center to the selection line:
        const float d = distanceFromLine ((**i).position(),
                                          camera.position(),
                                          direction);

        // if this vehicle-to-line distance is the smallest so far,
        // store it and this vehicle in the selection registers.
        if (d < minDistance)
        {
            minDistance = d;
            nearest = *i;
        }
    }

    return nearest;
}


// ----------------------------------------------------------------------------
// set a certain initial camera state used by several plug-ins


void 
OpenSteer::App::init3dCamera (AbstractVehicle& selected)
{
    init3dCamera (selected, cameraTargetDistance, camera2dElevation);
}

void 
OpenSteer::App::init3dCamera (AbstractVehicle& selected,
                              float distance,
                              float elevation)
{
    position3dCamera (selected, distance, elevation);
    camera.fixedDistDistance = distance;
    camera.fixedDistVOffset = elevation;
    camera.mode = Camera::cmFixedDistanceOffset;
}


void 
OpenSteer::App::init2dCamera (AbstractVehicle& selected)
{
    init2dCamera (selected, cameraTargetDistance, camera2dElevation);
}

void 
OpenSteer::App::init2dCamera (AbstractVehicle& selected,
                              float distance,
                              float elevation)
{
    position2dCamera (selected, distance, elevation);
    camera.fixedDistDistance = distance;
    camera.fixedDistVOffset = elevation;
    camera.mode = Camera::cmFixedDistanceOffset;
}


void 
OpenSteer::App::position3dCamera (AbstractVehicle& selected)
{
    position3dCamera (selected, cameraTargetDistance, camera2dElevation);
}

void 
OpenSteer::App::position3dCamera (AbstractVehicle& selected,
                                  float distance,
                                  float /*elevation*/)
{
    selectedVehicle = &selected;

    const Vec3 behind = selected.forward() * -distance;
    camera.setPosition (selected.position() + behind);
    camera.target = selected.position();
}


void 
OpenSteer::App::position2dCamera (AbstractVehicle& selected)
{
    position2dCamera (selected, cameraTargetDistance, camera2dElevation);
}

void 
OpenSteer::App::position2dCamera (AbstractVehicle& selected,
                                  float distance,
                                  float elevation)
{
    // position the camera as if in 3d:
    position3dCamera (selected, distance, elevation);

    // then adjust for 3d:
    Vec3 position3d = camera.position();
    position3d.y += elevation;
    camera.setPosition (position3d);
}


// ----------------------------------------------------------------------------
// camera updating utility used by several plug-ins


void 
OpenSteer::App::updateCamera (const float currentTime,
                              const float elapsedTime,
                              const AbstractVehicle& selected)
{
    camera.vehicleToTrack = &selected;
    camera.update (currentTime, elapsedTime, clock.getPausedState ());
}


// ----------------------------------------------------------------------------
// ground plane grid-drawing utility used by several plug-ins


void 
OpenSteer::App::gridUtility (const Vec3& gridTarget, bool withLines)
{
    // round off target to the nearest multiple of 2 (because the
    // checkboard grid with a pitch of 1 tiles with a period of 2)
    // then lower the grid a bit to put it under 2d annotation lines
    const Vec3 gridCenter ((round (gridTarget.x * 0.5) * 2),
                           (round (gridTarget.y * 0.5) * 2) - .05,
                           (round (gridTarget.z * 0.5) * 2));

    // colors for checkboard
    const Vec3 gray1 = grayColor (0.27);
    const Vec3 gray2 = grayColor (0.30);

    // draw 50x50 checkerboard grid with 50 squares along each side
    Draw::drawCheckerboardGrid (50, 50, gridCenter, gray1, gray2);

    // alternate style:
    if (withLines)
        Draw::drawLineGrid (50, 50, gridCenter, gBlack);
}


// ----------------------------------------------------------------------------
// draws a gray disk on the XZ plane under a given vehicle


void 
OpenSteer::App::highlightVehicleUtility (const AbstractVehicle& vehicle)
{
    drawXZDisk (vehicle.radius(), vehicle.position(), OpenSteer::gGray60, 20);
}


// ----------------------------------------------------------------------------
// draws a gray circle on the XZ plane under a given vehicle


void 
OpenSteer::App::circleHighlightVehicleUtility (const AbstractVehicle& vehicle)
{
    drawXZCircle (vehicle.radius () * 1.1, vehicle.position(), OpenSteer::gGray60, 20);
}


// ----------------------------------------------------------------------------
// draw a box around a vehicle aligned with its local space
// xxx not used as of 11-20-02


void 
OpenSteer::App::drawBoxHighlightOnVehicle (const AbstractVehicle& v, const Vec3 color)
{
    const float diameter = v.radius() * 2;
    const Vec3 size (diameter, diameter, diameter);
    Draw::drawBox (v, size, color);
}


// ----------------------------------------------------------------------------
// draws a colored circle (perpendicular to view axis) around the center
// of a given vehicle.  The circle's radius is the vehicle's radius times
// radiusMultiplier.


void 
OpenSteer::App::drawCircleHighlightOnVehicle (const AbstractVehicle& v,
                                              const Vec3 color,
                                              const float radiusMultiplier)
{
    const Vec3& cPosition = camera.position();
    draw3dCircle(v.radius() * radiusMultiplier,  // adjusted radius
                 v.position(),                   // center
                 v.position() - cPosition,       // view axis
                 color,                          // drawing color
                 20);                            // circle segments
}


// ----------------------------------------------------------------------------


void 
OpenSteer::App::printMessage (const char* message)
{
    std::cout << "App: " <<  message << std::endl << std::flush;
}


void 
OpenSteer::App::printMessage (const std::ostringstream& message)
{
    printMessage (message.str().c_str());
}


void 
OpenSteer::App::printWarning (const char* message)
{
    std::cout << "App: Warning: " <<  message << std::endl << std::flush;
}


void 
OpenSteer::App::printWarning (const std::ostringstream& message)
{
    printWarning (message.str().c_str());
}


// ------------------------------------------------------------------------
// print list of known commands
//
// XXX this list should be assembled automatically,
// XXX perhaps from a list of "command" objects created at initialization


void 
OpenSteer::App::keyboardMiniHelp (void)
{
    printMessage ("");
    printMessage ("defined single key commands:");
    printMessage ("  r      restart current PlugIn.");
    printMessage ("  s      select next vehicle.");
    printMessage ("  c      select next camera mode.");
    printMessage ("  f      select next preset frame rate");
    printMessage ("  Tab    select next PlugIn.");
    printMessage ("  a      toggle annotation on/off.");
    printMessage ("  Space  toggle between Run and Pause.");
    printMessage ("  ->     step forward one frame.");
    printMessage ("  Esc    exit.");
    printMessage ("");

    // allow PlugIn to print mini help for the function keys it handles
    selectedPlugIn->printMiniHelpForFunctionKeys ();
}


void 
OpenSteer::App::pushPhase (const int newPhase)
{
    // update timer for current (old) phase: add in time since last switch
    updatePhaseTimers ();
    // save old phase
    phaseStack[phaseStackIndex++] = phase;
    // set new phase
    phase = newPhase;
    // check for stack overflow
    if (phaseStackIndex >= phaseStackSize) errorExit ("phaseStack overflow");
}


void 
OpenSteer::App::popPhase (void)
{
    // update timer for current (old) phase: add in time since last switch
    updatePhaseTimers ();
    // restore old phase
    phase = phaseStack[--phaseStackIndex];
}


void 
OpenSteer::App::initPhaseTimers (void)
{
    phaseTimers[drawPhase] = 0;
    phaseTimers[updatePhase] = 0;
    phaseTimers[overheadPhase] = 0;
    phaseTimerBase = clock.getTotalRealTime ();
}


void 
OpenSteer::App::updatePhaseTimers (void)
{
    const float currentRealTime = clock.realTimeSinceFirstClockUpdate();
    phaseTimers[phase] += currentRealTime - phaseTimerBase;
    phaseTimerBase = currentRealTime;
}


// ----------------------------------------------------------------------------
