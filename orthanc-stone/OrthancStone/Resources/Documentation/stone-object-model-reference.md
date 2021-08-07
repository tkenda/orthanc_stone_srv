## Scene2D and viewport-related object reference

### `Scene2D` 

Represents a collection of layers that display 2D data.

These layers must implement `ISceneLayer`

The layers must be created externally and set to a specific Z-order index 
with the `SetLayer` method.

The `Scene2D` object merely acts as a layer container. It has no rendering 
or layer creation facility on its own.

The `Scene2D` contains an `AffineTransform2D` structure that defines how 
the various layer item coordinates are transformed before being displayed 
on the viewport (aka canvas)

It is up to each layer type-specific renderer to choose how this transformation
is used. See the various kinds of layer below for more details.

Examining the `Scene2D` contents can be done either by implementing the 
`Scene2D::IVisitor` interface and calling `Apply(IVisitor& visitor)` or by 
iterating between `GetMinDepth()` and `GetMaxDepth()` and calling the 
`ISceneLayer& GetLayer(int depth)` getter.

### `ISceneLayer` 

Interface that must be implemented by `Scene2D` layers. This is a closed list
that, as of 2020-03, contains:

```
  Type_InfoPanel,
  Type_ColorTexture,
  Type_Polyline,
  Type_Text,
  Type_FloatTexture,
  Type_LookupTableTexture
```

Please note that this interface mandates the implementation of a `GetRevision`
method returning an `uint64_t`. 

The idea is that when a model gets converted to a set of `ISceneLayer` 
instances, changes in the model that result in changes to the layers must 
increase the revision number of these layers.

That allows the rendering process to safely assume that a given layers whose
revision does not change hasn't been modified (this helps with caching).

Every mutable method in `ISceneLayer` instances that possibly change the visual
representation of an `ISceneLayer` must increase this revision number.

### Implementation: `FloatTextureSceneLayer`

Layer that renders an `Orthanc::ImageAccessor` object that must be convertible
to `Float32` image.

The constructor only uses the image accessor to perform a copy. It can safely 
be deleted afterwards.

The input values are mapped to the output values by taking into account various
properties that can be modified with:

- `SetWindowing`: uses windowing presets like "bone" or "lung"
- `SetCustomWindowing`: with manual window center and width
- `SetInverted`: toggles black <-> white inversion after windowing
- `SetApplyLog`: uses a non-linear response curve described in 
  https://theailearner.com/2019/01/01/log-transformation/ that expands contrast
  in dark areas while compressing contrast in bright ones. This is **not** 
  implemented in the OpenGL renderer!

The corresponding renderers are `OpenGLFloatTextureRenderer` and 
`CairoFloatTextureRenderer`. The scene transformation is applied during
rendering.

### Implementation: `ColorTextureSceneLayer`

Layer that renders an `Orthanc::ImageAccessor` object an RGBA image (alpha must
be premultiplied). 

The constructor only uses the image accessor to perform a copy. It can safely 
be deleted afterwards.

The corresponding renderers are `OpenGLColorTextureRenderer` and 
`CairoColorTextureRenderer`. The scene transformation is applied during
rendering.

### Implementation: `LookupTableTextureSceneLayer`

Layer that renders an `Orthanc::ImageAccessor` object that must be convertible
to `Float32` image.

The constructor only uses the image accessor to perform a copy. It can safely 
be deleted afterwards.

The final on-screen color of each pixel is determined by passing the input 
`Float32` value through a 256-entry look-up table (LUT) that can be passed as 
an array of either 256 x 3 bytes (for opaque RGB colors) or 256 x 4 bytes (for
RGBA pixels). The LUT is not specified at construction time, but with 
calls to `SetLookupTable` or `SetLookupTableGrayscale` (that fills the LUT 
with a gradient from black to white, fully opaque)

The range of input values that is mapped to the entirety of the LUT is, by
default, the full image range, but can be customized with `SetRange`.

The corresponding renderers are `OpenGLLookupTableTextureRenderer` and
`CairoLookupTableTextureRenderer`. The scene transformation is applied during
rendering.

### Implementation: `PolylineSceneLayer`

Layer that renders vector-based polygonal lines. 

Polylines can be added with the `AddChain` method, that accepts a `Chain`, that
is a typedef to `std::vector<ScenePoint2D>`, a flag to specify whether the 
chain must be automatically close (last point of the vector connected to the
first one) and the chain color (a `Color` structure).

Please note that the line thickness is, contrary to the color, specified 
per-chain but rather per-layer. 

If you need multiple line thicknesses, multiple `PolylineSceneLayer` must be
created.

The corresponding renderers are `OpenGLAdvancedPolylineRenderer` and
`CairoPolylineRenderer`. The scene transformation is applied during
rendering.

### Implementation: `TextSceneLayer`

This layers renders a paragraph of text. 

The inputs to the layer can be changed after creation and are:
- The text iself, supplied as an UTF-8 encoded string in `SetText`
- The font used for rendering, set by `SetFontIndex`.
- The text anchoring, through `SetAnchor`: the text can be anchored to 
  various positions, such as top lef, center, bottom center,... These 
  various anchors are part of the `BitmapAnchor` enumeration.
- The text position, relative to its anchor, through `SetPosition`.

The font is supplied as an index. This is an index in the set of fonts 
that has been registered in the viewport compositor. The following code 
shows how to set such a font:

```
std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport_.Lock());
lock->GetCompositor().SetFont(0, 
                              Orthanc::EmbeddedResources::UBUNTU_FONT, 
                              32, Orthanc::Encoding_Latin1);
// where 32 is the font size in pixels
```

This call uses the embedded `UBUNTU_FONT` resource that has been defined in
the `CMakeLists.txt` file with:

```
EmbedResources(
  UBUNTU_FONT  ${CMAKE_BINARY_DIR}/ubuntu-font-family-0.83/Ubuntu-R.ttf
)
```

Please note that you must supply a font: there is no default font provided by
the OpenGL or Cairo compositors.

The corresponding renderers are `OpenGLTextRenderer` and
`CairoTextRenderer`. The scene transformation is not applied during rendering,
because the text anchoring, position and scaling are computed relative to the
viewport/canvas.

### Implementation: `InfoPanelSceneLayer`

This layer is designed to display an image, supplied through an 
`Orthanc::ImageAccessor` reference (only used at construction time).

The image is not transformed according to the normal layer transformation but 
is rather positioned relative to the canvas, with the same mechanism as the
`TextSceneLayer` described above.

The image position is specified with the sole means of the `SetAnchor` method.

The corresponding renderers are `OpenGLInfoPanelRenderer` and 
`CairoInfoPanelRenderer`.

### `IViewport`

https://bitbucket.org/sjodogne/orthanc-stone/src/broker/Framework/Viewport/IViewport.h

(**not** the one in `Deprecated`)
- Implemented by classes that:
  - manage the on-screen display of a `Scene2D` trough a compositor.
  - Own the `ICompositor` object that performs the rendering. 
  - Own the `Scene2D` (TODO: currently through `ViewportController` --> `Scene2D`)
  - Provide a `Lock` method that returns a RAII, that must be kept alive when 
    modifying the underlying objects (controller, compositor, scene), but not 
    longer.

#### Implementation: `SdlOpenGLViewport`
- Implementation of a viewport rendered on a SDL window, that uses OpenGL for 
  rendering.
- Instantiating this object creates an SDL window. Automatic scaling for hiDPI
  displays can be toggled on or off. 

#### Implementation: `WebGLViewport`
- Implementation of a viewport rendered on a DOM canvas, that uses OpenGL for 
  rendering.
- Contrary to the SDL OpenGL viewport, the canvas must already be existing 
  when the ctor is called.

### `ICompositor`
The interface providing a rendering service for `Scene2D` objects. 

**Subclasses:** `CairoCompositor`, `OpenGLCompositor`
 
You do not need to create compositor instances. They are created for you when
instantiating a viewport.

### `ViewportController`
This concrete class is instantiated by its `IViewport` owner. 

**TODO:** its functionality is not well defined and should be moved into the 
viewport base class. Support for measuring tools should be moved to a special
interactor.

- contains:
  - array of `MeasureTool`
  - ref to `IViewport`
  - `activeTracker_`
  - owns a `Scene2D`
  - weak ref to `UndoStack`
  - cached `canvasToSceneFactor_`

- contains logic to:
  - pass commands to undostack (trivial)
  - logic to locate `MeasureTool` in the HitTest
  - OTOH, the meat of the measuring tool logic (highlighting etc..) is 
    done in app-specific code (`VolumeSlicerWidget`)
  - accept new Scene transform and notify listeners
  - **the code that uses the interactor** (`HandleMousePress`) is only 
    called by the new `WebAssemblyViewport` !!! **TODO** clean this mess

### `IViewportInteractor`
- must provide logic to respond to `CreateTracker`

### `DefaultViewportInteractor`
- provides Pan+Rotate+Zoom trackers

### `WebGLViewportsRegistry`

This class is a singleton (accessible through `GetWebGLViewportsRegistry()` 
that deals with context losses in the WebGL contexts.

You use it by creating a WebGLViewport in the following fashion:

```
boost::shared_ptr<OrthancStone::WebGLViewport> viewport(
  OrthancStone::GetWebGLViewportsRegistry().Add(canvasId));
```

## Source data related

### `IVolumeSlicer`

A very simple interface with a single method:
`IVolumeSlicer::IExtractedSlice* ExtractSlice(const CoordinateSystem3D& cuttingPlane)`

### `IVolumeSlicer::IExtractedSlice`

On a slice has been extracted from a volume by an `IVolumeSlicer`, it can
report its *revision number*. 

If another call to `ExtractSlice` with the same cutting plane is made, but 
the returned slice revision is different, it means that the volume has 
changed and the scene layer must be refreshed.

Please see `VolumeSceneLayerSource::Update` to check how this logic is 
implemented.


### `OrthancSeriesVolumeProgressiveLoader`

This class implements `IVolumeSlicer` (and `IObservable`) and can be used to 
load a volume stored in a Dicom series on an Orthanc server.

Over the course of the series loading, various notifications are sent:

The first one is `OrthancStone::DicomVolumeImage::GeometryReadyMessage` that
is sent when the volume extent and geometric properties are known.

Then, as slices get loaded and the volume is filled,  
`OrthancStone::DicomVolumeImage::ContentUpdatedMessage` are sent.

Once all the highest-quality slices have been loaded, the 
`OrthancSeriesVolumeProgressiveLoader::VolumeImageReadyInHighQuality` 
notification is sent.

Please note that calling `ExtractSlice` *before* the geometry is loaded will
yield an instance of `InvalidSlice` that cannot be used to create a layer.

On the other hand, 

### `VolumeSceneLayerSource`

This class makes the bridge between a volume (supplied by an `IVolumeSlicer`
interface) and a `Scene2D`. 

Please note that the bulk of the work is done the objects implementing 
`IVolumeSlicer` and this object merely connects things together.

For instance, deciding whether an image (texture) or vector (polyline) layer
is done by the `IVolumeSlicer` implementation.

- contains:
  - reference to Scene2D
  - `layerIndex_` (fixed at ctor) that is the index, in the Scene2D layer 
    stack, of the layer that will be created/updated
  - `IVolumeSlicer`

- contains logic to:
  - extract a slice from the slicer and set/refresh the Scene2D layer at 
    the supplied `layerIndex_`
  - refresh this based on the slice revision or configuration revision
  - accept a configuration that will be applied to the layer
  - the `Update()` method will 

## Updates and the configurators

`ISceneLayer` does not expose mutable methods. 

The way to change a layer once it has been created is through configurator 
objets.

If you plan to set (even only once) or modify some layer properties after 
layer creation, you need to create a matching configurator objet.

For instance, in the `VolumeSceneLayerSource`, the `SetConfigurator` method
will store a `ILayerStyleConfigurator* configurator_`.

In the `OrthancView` ctor, you can see how it is used:

```
std::unique_ptr<GrayscaleStyleConfigurator> style(
  new GrayscaleStyleConfigurator);

style->SetLinearInterpolation(true);

...<some more code>...

std::unique_ptr<LookupTableStyleConfigurator> config(
  new LookupTableStyleConfigurator);

config->SetLookupTable(Orthanc::EmbeddedResources::COLORMAP_HOT);

```

The configurator type are created according to the type of layer.¸

Later, in `VolumeSceneLayerSource::Update(const CoordinateSystem3D& plane)`, 
if the cutting plane has **not** changed and if the layer revision has **not**
changed, we test `configurator_->GetRevision() != lastConfiguratorRevision_` 
and, if different, we call `configurator_->ApplyStyle(scene_.GetLayer(layerDepth_));`

This allows to change layer properties that do not depend on the layer model
contents.

On the other hand, if the layer revision has changed, when compared to the 
last time it has been rendered (stored in `lastRevision_`), then we need to 
ask the slice to create a brand new layer.

Another way to see it is that layer rendering depend on model data and view 
data. The model data is not mutable in the layer and, if the model changes, the 
layer must be recreated.

If only the view properties change (the configurator), we call ApplyStyle 
(that **will** mutate some of the layer internals)

Please note that the renderer does **not** know about the configurator : the 
renderer uses properies in the layer and does not care whether those have 
been set once at construction time or at every frame (configuration time).


## Cookbook

### Simple application

#### Building

In order to create a Stone application, you need to:

- CMake-based application:
    ```
    include(${STONE_SOURCES_DIR}/Resources/CMake/OrthancStoneConfiguration.cmake)
    ```
  with this library target that you have to define:
    ```
    add_library(OrthancStone STATIC ${ORTHANC_STONE_SOURCES})
    ```
  then link with this library:
    ```
    target_link_libraries(MyStoneApplication OrthancStone)
    ```

Building is supported with emscripten, Visual C++ (>= 9.0), gcc...

emscripten recommended version >= 1.38.41

These are very rough guidelines. See the `Samples` folder for actual examples.

#### Structure

The code requires a loader (object that )

Initialize:

```
Orthanc::Logging::Initialize();
Orthanc::Logging::EnableInfoLevel(true);
```
Call, in WASM:
```
DISPATCH_JAVASCRIPT_EVENT("StoneInitialized");
```

# Notes

- It is NOT possible to abandon the existing loaders : they contain too much loader-specific getters



