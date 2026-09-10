#pragma once
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>
import Graphics.Scene.Models.Factory;

class W3DRenderObject;
class ChunkLoadClass;
Graphics::ModelFactory<W3DRenderObject>* Load_ParticleEmitter_Factory(ChunkLoadClass& source);
