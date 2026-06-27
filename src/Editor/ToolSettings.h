#pragma once

struct ToolSettings {
  enum class BrushShape { Round, Square, ForwardSlash, BackSlash };
  enum class BackgroundMode { Opaque, Transparent };
  enum class FillMode { Outline, Opaque, Fill };
  float strokeWidth = 1.0f;
  float lineWidth = 1.0f;

  BrushShape brushShape = BrushShape::Round;
  FillMode fillmode = FillMode::Outline;
  bool useBackgroundColor = false;

  float eraserSize = 4.0f;
  float airbrushRadius = 12.0f;
  int airbrushDensity = 25;
  int zoomLevel = 1;

  BackgroundMode backgroundMode = BackgroundMode::Opaque;
};
