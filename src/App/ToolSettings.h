#pragma once

struct ToolSettings {
  enum class BrushShape { Round, Square, ForwardSlash, BackSlash };
  enum class BackgroundMode { Opaque, Transparent };

  float strokeWidth = 1.0f;
  float lineWidth = 1.0f;

  BrushShape brushShape = BrushShape::Round;
  bool useBackgroundColor = false;

  float eraserSize = 4.0f;
  float airbrushRadius = 12.0f;
  int airbrushDensity = 25;
  int zoomLevel = 1;

  BackgroundMode backgroundMode = BackgroundMode::Opaque;
};
