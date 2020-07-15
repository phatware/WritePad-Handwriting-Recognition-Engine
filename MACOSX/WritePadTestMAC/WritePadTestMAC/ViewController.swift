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
    
    @IBOutlet var result1 : NSTextField!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        RecognizerManager.resetRecognizerOptions()
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

