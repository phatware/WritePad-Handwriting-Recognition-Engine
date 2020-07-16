//
//  NSInkView.swift
//  WritePadTestMAC
//
//  Created by Stan Miasnikov on 7/15/20.
//

import Cocoa
import WritePadMAC

class NSInkView: NSView {

    var inlineReco : InlineRecognizer = InlineRecognizer()

    var trackingArea : NSTrackingArea?
    var trackMouse : Bool = false

    override func updateTrackingAreas() {
        if trackingArea != nil {
            self.removeTrackingArea(trackingArea!)
        }
        let options : NSTrackingArea.Options =
            [.mouseEnteredAndExited, .mouseMoved, .activeInKeyWindow, .enabledDuringMouseDrag]
        trackingArea = NSTrackingArea(rect: self.bounds, options: options,
                                      owner: self, userInfo: nil)
        self.addTrackingArea(trackingArea!)
    }
    
    func clear() {
        inlineReco.empty()
        setNeedsDisplay(self.bounds)
    }

    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)
        // Drawing code here.
        if inlineReco.strokeLen > 1 {
            renderLine(inlineReco.ptStroke, pointCount: inlineReco.strokeLen)
        }
        let cnt = inlineReco.strokeCount()
        if cnt > 0 {
            for i in 0...cnt-1 {
                if let arr = inlineReco.getStrokePoints(i) {
                    renderLine(arr)
                }
            }
        }
    }
    
    fileprivate func penLocation(_ touch : NSTouch) -> CGTracePoint {
        let location : CGTracePoint = CGTracePoint(pt: touch.location(in: self), pressure: 100)
        return location
    }
    
    override func mouseDown(with event: NSEvent) {
        var pt = event.locationInWindow
        pt = convert(pt, from: nil)
        let location : CGTracePoint = CGTracePoint(pt: pt, pressure: 100)
        inlineReco.processTouch(atLocation: location)
        //trackMouse = true
    }

    override func mouseUp(with event: NSEvent) {
        //trackMouse = false
        var pt = event.locationInWindow
        pt = convert(pt, from: nil)
        let location : CGTracePoint = CGTracePoint(pt: pt, pressure: 100)
        inlineReco.processTouchUp(atLocation: location, touchCount: 1)
        setNeedsDisplay(self.bounds)
    }
    
    override func mouseDragged(with event: NSEvent) {
        // print("Mouse dragged: \(event)")
        var pt = event.locationInWindow
        pt = convert(pt, from: nil)
        let location : CGTracePoint = CGTracePoint(pt: pt, pressure: 100)
        inlineReco.processMove(toLocation: location)
        setNeedsDisplay(self.bounds)
    }

    override func mouseMoved(with event: NSEvent) {
    }
    
    override func touchesBegan(with event: NSEvent) {

        let touches = event.touches(for: self)
        if let touch = touches.first {
            let location = penLocation(touch)
            inlineReco.processTouch(atLocation: location)
        }
    }

    override func touchesMoved(with event: NSEvent) {
        
        let touches = event.touches(for: self)
        if let touch = touches.first {
            let location = penLocation(touch)
            inlineReco.processMove(toLocation: location)
        }
    }

    override func touchesEnded(with event: NSEvent) {
     
        let touches = event.touches(for: self)
        if let touch = touches.first {
            let location = penLocation(touch)
            inlineReco.processTouchUp(atLocation: location, touchCount: 1)
        }
    }
    
    override func touchesCancelled(with event: NSEvent) {
        
        let touches = event.touches(for: self)
        if let touch = touches.first {
            let location = penLocation(touch)
            inlineReco.processCancel(atLocation: location)
        }
    }
    
    fileprivate func renderLine(_ points : CGStroke, pointCount : Int32 ) {
        
        NSColor.blue.set() // choose color
        let figure = NSBezierPath() // container for line(s)
        
        figure.lineCapStyle = .round
        figure.lineJoinStyle = .round
        figure.flatness = 0.5
        figure.lineWidth = 1.5  // hair line
        figure.stroke()  // draw line(s) in color
        
        let pt : CGPoint = points[0].pt
        figure.move(to: pt) // start point
        // figure.lineToPoint(NSMakePoint(x+10.0, y+10.0)) // destination

        for i in 1...pointCount-1 {
            // figure.curve(to: points[i+2].pt, controlPoint1: points[i].pt, controlPoint2: points[i+1].pt)
            figure.line(to: points[Int(i)].pt)
        }
        figure.stroke()
    }

    fileprivate func renderLine(_ points : [NSValue] ) {
        
        if points.count > 0 {
            NSColor.blue.set() // choose color
            let figure = NSBezierPath() // container for line(s)
            
            figure.lineCapStyle = .round
            figure.lineJoinStyle = .round
            figure.flatness = 0.5
            figure.lineWidth = 1.5  // hair line
            figure.stroke()  // draw line(s) in color
            
            var pt : CGPoint = points[0].pointValue
            figure.move(to: pt) // start point
            if points.count == 1 {
                pt.x = pt.x + 1;
                figure.line(to: pt)
            }
            else {
                for i in 1...points.count-1 {
                    figure.line(to: points[i].pointValue)
                }
            }
            figure.stroke()
        }
    }
}

