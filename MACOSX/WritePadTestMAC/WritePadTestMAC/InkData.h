
#pragma once

#import <Foundation/Foundation.h>

typedef struct {
    CGPoint * stroke;
    int length;
} TEST_STROKES;

extern TEST_STROKES * testStrokes;
extern const UInt numTestStrokes;
