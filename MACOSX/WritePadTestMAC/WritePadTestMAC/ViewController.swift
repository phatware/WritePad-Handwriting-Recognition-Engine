//
//  ViewController.swift
//  WritePadTestMAC
//
//  Created by Stan Miasnikov on 7/12/20.
//

import Cocoa
import WritePadMAC

class ViewController: NSViewController {
    
    var reco : RecognizerManager = RecognizerManager.shared()
    
    @IBOutlet var inkView : NSInkView!
    @IBOutlet var result1 : NSTextField!
    @IBOutlet var result2 : NSTextField!

    override func viewDidLoad() {
        super.viewDidLoad()
        
        
        inkView.allowedTouchTypes = [NSTouch.TouchTypeMask.direct, NSTouch.TouchTypeMask.indirect];
        RecognizerManager.resetRecognizerOptions()
        inkView.inlineReco.delegate = self
        // Do any additional setup after loading the view.
    }

    override var representedObject: Any? {
        didSet {
        // Update the view, if already loaded.
        }
    }
        
    @IBAction func recognizeFromData(_ sender : Any?) {
        self.recognizeFromData()
    }
    
    @IBAction func clear(_ sender : Any?) {
        inkView.clear()
        result2.stringValue = ""
    }

    func recognizeFromData() {
        
        let str : UnsafePointer<TEST_STROKES> = UnsafePointer<TEST_STROKES>(testStrokes)
        self.reco.reset()
        
        for i in 0...numTestStrokes {
            self.reco.addPoints(str[Int(i)].stroke, length: str[Int(i)].length)
        }
        
        if self.reco.recognize() {
            if let result = self.reco.getResult() {
                print("Recognition result: \(result)")
                result1.stringValue = result
            }
        }
    }
}

extension ViewController: InlineRecognizerProtocol
{
    func inkCollectorResultReady(_ inkView: InlineRecognizer, theResult string: String) {
        
    }
    
    func inkCollectorAsyncResultReady(_ inkView: InlineRecognizer, theResult string: String) {
        result2.stringValue = string
    }
    
    func inkCollectorRecognizedGesture(_ inkView: InlineRecognizer, withGesture gesture: GESTURE_TYPE, isEmpty bEmpty: Bool) -> Bool {
        return false
    }
}
