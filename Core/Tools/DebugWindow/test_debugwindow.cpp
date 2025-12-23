/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// TheSuperHackers: Test program for DebugWindow wxWidgets conversion

#include "debugwindow_wx.h"
#include <iostream>
#include <thread>
#include <chrono>

int main()
{
    std::cout << "Testing DebugWindow wxWidgets conversion..." << std::endl;
    
    // Create the debug dialog
    CreateDebugDialog();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Test basic functionality
    AppendMessage("DebugWindow test started");
    SetFrameNumber(1);
    
    AdjustVariable("TestVar1", "42");
    AdjustVariable("TestVar2", "Hello World");
    
    // Simulate a game loop
    for (int frame = 1; frame <= 100; ++frame) {
        SetFrameNumber(frame);
        
        if (frame % 10 == 0) {
            char msg[64];
            snprintf(msg, sizeof(msg), "Frame %d reached", frame);
            AppendMessage(msg);
            
            char var[32];
            snprintf(var, sizeof(var), "%d", frame * 10);
            AdjustVariable("FrameCounter", var);
        }
        
        // Check if we can continue
        while (!CanAppContinue()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Simulate frame time
        if (RunAppFast()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));  // 10x speed
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Normal speed
        }
    }
    
    AppendMessage("Test completed successfully!");
    
    // Keep running until user closes
    std::cout << "Test running... Close the debug window to exit." << std::endl;
    while (CanAppContinue()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    // Clean up
    DestroyDebugDialog();
    
    std::cout << "DebugWindow test finished." << std::endl;
    return 0;
}