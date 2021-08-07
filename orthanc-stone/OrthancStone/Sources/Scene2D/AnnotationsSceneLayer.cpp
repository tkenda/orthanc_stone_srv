/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 **/


#include "AnnotationsSceneLayer.h"

#include "MacroSceneLayer.h"
#include "PolylineSceneLayer.h"
#include "TextSceneLayer.h"

#include <OrthancException.h>

#include <boost/math/constants/constants.hpp>
#include <list>

static const double HANDLE_SIZE = 10.0;
static const double PI = boost::math::constants::pi<double>();

static const char* const KEY_ANNOTATIONS = "annotations";
static const char* const KEY_TYPE = "type";
static const char* const KEY_X1 = "x1";
static const char* const KEY_Y1 = "y1";
static const char* const KEY_X2 = "x2";
static const char* const KEY_Y2 = "y2";
static const char* const KEY_X3 = "x3";
static const char* const KEY_Y3 = "y3";
static const char* const KEY_UNITS = "units";

static const char* const VALUE_ANGLE = "angle";
static const char* const VALUE_CIRCLE = "circle";
static const char* const VALUE_SEGMENT = "segment";
static const char* const VALUE_MILLIMETERS = "millimeters";
static const char* const VALUE_PIXELS = "pixels";

#if 0
static OrthancStone::Color COLOR_PRIMITIVES(192, 192, 192);
static OrthancStone::Color COLOR_HOVER(0, 255, 0);
static OrthancStone::Color COLOR_TEXT(255, 0, 0);
#else
static OrthancStone::Color COLOR_PRIMITIVES(0x40, 0x82, 0xad);
static OrthancStone::Color COLOR_HOVER(0x40, 0xad, 0x79);
static OrthancStone::Color COLOR_TEXT(0x4e, 0xde, 0x99);
#endif


namespace OrthancStone
{
  class AnnotationsSceneLayer::GeometricPrimitive : public boost::noncopyable
  {
  private:
    bool         modified_;
    Annotation&  parentAnnotation_;
    Color        color_;
    Color        hoverColor_;
    bool         isHover_;
    int          depth_;

  public:
    GeometricPrimitive(Annotation& parentAnnotation,
                       int depth) :
      modified_(true),
      parentAnnotation_(parentAnnotation),
      color_(COLOR_PRIMITIVES),
      hoverColor_(COLOR_HOVER),
      isHover_(false),
      depth_(depth)
    {
    }
      
    virtual ~GeometricPrimitive()
    {
    }

    Annotation& GetParentAnnotation() const
    {
      return parentAnnotation_;
    }
      
    int GetDepth() const
    {
      return depth_;
    }

    void SetHover(bool hover)
    {
      if (hover != isHover_)
      {
        isHover_ = hover;
        modified_ = true;
      }
    }

    bool IsHover() const
    {
      return isHover_;
    }
      
    void SetModified(bool modified)
    {
      modified_ = modified;
    }
      
    bool IsModified() const
    {
      return modified_;
    }

    void SetColor(const Color& color)
    {
      SetModified(true);
      color_ = color;
    }

    void SetHoverColor(const Color& color)
    {
      SetModified(true);
      hoverColor_ = color;
    }

    const Color& GetColor() const
    {
      return color_;
    }

    const Color& GetHoverColor() const
    {
      return hoverColor_;
    }

    virtual bool IsHit(const ScenePoint2D& p,
                       const Scene2D& scene) const = 0;

    // Always called, even if not modified
    virtual void RenderPolylineLayer(PolylineSceneLayer& polyline,
                                     const Scene2D& scene) = 0;

    // Only called if modified
    virtual void RenderOtherLayers(MacroSceneLayer& macro,
                                   const Scene2D& scene) = 0;

    virtual void MovePreview(const ScenePoint2D& delta) = 0;

    virtual void MoveDone(const ScenePoint2D& delta) = 0;
  };
    

  class AnnotationsSceneLayer::Annotation : public boost::noncopyable
  {
  private:
    typedef std::list<GeometricPrimitive*>  GeometricPrimitives;
      
    AnnotationsSceneLayer&  that_;
    GeometricPrimitives     primitives_;
    Units                   units_;
      
  public:
    explicit Annotation(AnnotationsSceneLayer& that,
                        Units units) :
      that_(that),
      units_(units)
    {
      that.AddAnnotation(this);
    }
      
    virtual ~Annotation()
    {
      for (GeometricPrimitives::iterator it = primitives_.begin(); it != primitives_.end(); ++it)
      {
        that_.DeletePrimitive(*it);
      }
    }

    Units GetUnits() const
    {
      return units_;
    }

    GeometricPrimitive* AddPrimitive(GeometricPrimitive* primitive)
    {
      if (primitive == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
      }
      else
      {
        assert(that_.primitives_.find(primitive) == that_.primitives_.end());
        primitives_.push_back(primitive);  // For automated deallocation
        that_.primitives_.insert(primitive);
        return primitive;
      }
    }

    template <typename T>
    T& AddTypedPrimitive(T* primitive)
    {
      AddPrimitive(primitive);
      return *primitive;
    }

    virtual void SignalMove(GeometricPrimitive& primitive) = 0;

    virtual void Serialize(Json::Value& target) = 0;
  };


  class AnnotationsSceneLayer::Handle : public GeometricPrimitive
  {
  private:
    ScenePoint2D  center_;
    ScenePoint2D  delta_;

  public:
    explicit Handle(Annotation& parentAnnotation,
                    const ScenePoint2D& center) :
      GeometricPrimitive(parentAnnotation, 0),  // Highest priority
      center_(center),
      delta_(0, 0)
    {
    }

    void SetSize(unsigned int size)
    {
      SetModified(true);
    }

    void SetCenter(const ScenePoint2D& center)
    {
      SetModified(true);
      center_ = center;
      delta_ = ScenePoint2D(0, 0);
    }

    ScenePoint2D GetCenter() const
    {
      return center_ + delta_;
    }

    virtual bool IsHit(const ScenePoint2D& p,
                       const Scene2D& scene) const ORTHANC_OVERRIDE
    {
      const double zoom = scene.GetSceneToCanvasTransform().ComputeZoom();

      double dx = (center_.GetX() + delta_.GetX() - p.GetX()) * zoom;
      double dy = (center_.GetY() + delta_.GetY() - p.GetY()) * zoom;

      return (std::abs(dx) <= HANDLE_SIZE / 2.0 &&
              std::abs(dy) <= HANDLE_SIZE / 2.0);
    }

    virtual void RenderPolylineLayer(PolylineSceneLayer& polyline,
                                     const Scene2D& scene) ORTHANC_OVERRIDE
    {
      const double zoom = scene.GetSceneToCanvasTransform().ComputeZoom();

      // TODO: take DPI into account 
      double x1 = center_.GetX() + delta_.GetX() - (HANDLE_SIZE / 2.0) / zoom;
      double y1 = center_.GetY() + delta_.GetY() - (HANDLE_SIZE / 2.0) / zoom;
      double x2 = center_.GetX() + delta_.GetX() + (HANDLE_SIZE / 2.0) / zoom;
      double y2 = center_.GetY() + delta_.GetY() + (HANDLE_SIZE / 2.0) / zoom;

      PolylineSceneLayer::Chain chain;
      chain.reserve(4);
      chain.push_back(ScenePoint2D(x1, y1));
      chain.push_back(ScenePoint2D(x2, y1));
      chain.push_back(ScenePoint2D(x2, y2));
      chain.push_back(ScenePoint2D(x1, y2));

      if (IsHover())
      {
        polyline.AddChain(chain, true /* closed */, GetHoverColor());
      }
      else
      {
        polyline.AddChain(chain, true /* closed */, GetColor());
      }
    }
      
    virtual void RenderOtherLayers(MacroSceneLayer& macro,
                                   const Scene2D& scene) ORTHANC_OVERRIDE
    {
    }

    virtual void MovePreview(const ScenePoint2D& delta) ORTHANC_OVERRIDE
    {
      SetModified(true);
      delta_ = delta;
      GetParentAnnotation().SignalMove(*this);
    }

    virtual void MoveDone(const ScenePoint2D& delta) ORTHANC_OVERRIDE
    {
      SetModified(true);
      center_ = center_ + delta;
      delta_ = ScenePoint2D(0, 0);
      GetParentAnnotation().SignalMove(*this);
    }
  };

    
  class AnnotationsSceneLayer::Segment : public GeometricPrimitive
  {
  private:
    ScenePoint2D  p1_;
    ScenePoint2D  p2_;
    ScenePoint2D  delta_;
      
  public:
    Segment(Annotation& parentAnnotation,
            const ScenePoint2D& p1,
            const ScenePoint2D& p2) :
      GeometricPrimitive(parentAnnotation, 1),  // Can only be selected if no handle matches
      p1_(p1),
      p2_(p2),
      delta_(0, 0)
    {
    }

    void SetPosition(const ScenePoint2D& p1,
                     const ScenePoint2D& p2)
    {
      SetModified(true);
      p1_ = p1;
      p2_ = p2;
      delta_ = ScenePoint2D(0, 0);
    }

    ScenePoint2D GetPosition1() const
    {
      return p1_ + delta_;
    }

    ScenePoint2D GetPosition2() const
    {
      return p2_ + delta_;
    }

    virtual bool IsHit(const ScenePoint2D& p,
                       const Scene2D& scene) const ORTHANC_OVERRIDE
    {
      const double zoom = scene.GetSceneToCanvasTransform().ComputeZoom();
      return (ScenePoint2D::SquaredDistancePtSegment(p1_ + delta_, p2_ + delta_, p) * zoom * zoom <=
              (HANDLE_SIZE / 2.0) * (HANDLE_SIZE / 2.0));
    }

    virtual void RenderPolylineLayer(PolylineSceneLayer& polyline,
                                     const Scene2D& scene) ORTHANC_OVERRIDE
    {
      PolylineSceneLayer::Chain chain;
      chain.reserve(2);
      chain.push_back(p1_ + delta_);
      chain.push_back(p2_ + delta_);

      if (IsHover())
      {
        polyline.AddChain(chain, false /* closed */, GetHoverColor());
      }
      else
      {
        polyline.AddChain(chain, false /* closed */, GetColor());
      }
    }
      
    virtual void RenderOtherLayers(MacroSceneLayer& macro,
                                   const Scene2D& scene) ORTHANC_OVERRIDE
    {
    }

    virtual void MovePreview(const ScenePoint2D& delta) ORTHANC_OVERRIDE
    {
      SetModified(true);
      delta_ = delta;
      GetParentAnnotation().SignalMove(*this);
    }

    virtual void MoveDone(const ScenePoint2D& delta) ORTHANC_OVERRIDE
    {
      SetModified(true);
      p1_ = p1_ + delta;
      p2_ = p2_ + delta;
      delta_ = ScenePoint2D(0, 0);
      GetParentAnnotation().SignalMove(*this);
    }
  };

    
  class AnnotationsSceneLayer::Circle : public GeometricPrimitive
  {
  private:
    ScenePoint2D  p1_;
    ScenePoint2D  p2_;
    ScenePoint2D  delta_;
      
  public:
    Circle(Annotation& parentAnnotation,
           const ScenePoint2D& p1,
           const ScenePoint2D& p2) :
      GeometricPrimitive(parentAnnotation, 2),
      p1_(p1),
      p2_(p2),
      delta_(0, 0)
    {
    }

    void SetPosition(const ScenePoint2D& p1,
                     const ScenePoint2D& p2)
    {
      SetModified(true);
      p1_ = p1;
      p2_ = p2;
      delta_ = ScenePoint2D(0, 0);
    }

    ScenePoint2D GetPosition1() const
    {
      return p1_ + delta_;
    }

    ScenePoint2D GetPosition2() const
    {
      return p2_ + delta_;
    }

    virtual bool IsHit(const ScenePoint2D& p,
                       const Scene2D& scene) const ORTHANC_OVERRIDE
    {
      const double zoom = scene.GetSceneToCanvasTransform().ComputeZoom();

      ScenePoint2D middle((p1_.GetX() + p2_.GetX()) / 2.0,
                          (p1_.GetY() + p2_.GetY()) / 2.0);
        
      const double radius = ScenePoint2D::DistancePtPt(middle, p1_);
      const double distance = ScenePoint2D::DistancePtPt(middle, p - delta_);

      return std::abs(radius - distance) * zoom <= HANDLE_SIZE / 2.0;
    }

    virtual void RenderPolylineLayer(PolylineSceneLayer& polyline,
                                     const Scene2D& scene) ORTHANC_OVERRIDE
    {
      static unsigned int NUM_SEGMENTS = 128;

      ScenePoint2D middle((p1_.GetX() + p2_.GetX()) / 2.0,
                          (p1_.GetY() + p2_.GetY()) / 2.0);
        
      const double radius = ScenePoint2D::DistancePtPt(middle, p1_);

      double increment = 2.0 * PI / static_cast<double>(NUM_SEGMENTS - 1);

      PolylineSceneLayer::Chain chain;
      chain.reserve(NUM_SEGMENTS);

      double theta = 0;
      for (unsigned int i = 0; i < NUM_SEGMENTS; i++)
      {
        chain.push_back(ScenePoint2D(delta_.GetX() + middle.GetX() + radius * cos(theta),
                                     delta_.GetY() + middle.GetY() + radius * sin(theta)));
        theta += increment;
      }
        
      if (IsHover())
      {
        polyline.AddChain(chain, false /* closed */, GetHoverColor());
      }
      else
      {
        polyline.AddChain(chain, false /* closed */, GetColor());
      }
    }
      
    virtual void RenderOtherLayers(MacroSceneLayer& macro,
                                   const Scene2D& scene) ORTHANC_OVERRIDE
    {
    }

    virtual void MovePreview(const ScenePoint2D& delta) ORTHANC_OVERRIDE
    {
      SetModified(true);
      delta_ = delta;
      GetParentAnnotation().SignalMove(*this);
    }

    virtual void MoveDone(const ScenePoint2D& delta) ORTHANC_OVERRIDE
    {
      SetModified(true);
      p1_ = p1_ + delta;
      p2_ = p2_ + delta;
      delta_ = ScenePoint2D(0, 0);
      GetParentAnnotation().SignalMove(*this);
    }
  };

    
  class AnnotationsSceneLayer::Arc : public GeometricPrimitive
  {
  private:
    ScenePoint2D  start_;
    ScenePoint2D  middle_;
    ScenePoint2D  end_;
    double        radius_;  // in pixels

    void ComputeAngles(double& fullAngle,
                       double& startAngle,
                       double& endAngle) const
    {
      const double x1 = start_.GetX();
      const double y1 = start_.GetY();
      const double xc = middle_.GetX();
      const double yc = middle_.GetY();
      const double x2 = end_.GetX();
      const double y2 = end_.GetY();
        
      startAngle = atan2(y1 - yc, x1 - xc);
      endAngle = atan2(y2 - yc, x2 - xc);

      fullAngle = endAngle - startAngle;
        
      while (fullAngle < -PI)
      {
        fullAngle += 2.0 * PI;
      }
        
      while (fullAngle >= PI)
      {
        fullAngle -= 2.0 * PI;
      }
    }
      
  public:
    Arc(Annotation& parentAnnotation,
        const ScenePoint2D& start,
        const ScenePoint2D& middle,
        const ScenePoint2D& end) :
      GeometricPrimitive(parentAnnotation, 2),
      start_(start),
      middle_(middle),
      end_(end),
      radius_(20)
    {
    }

    double GetAngle() const
    {
      double fullAngle, startAngle, endAngle;
      ComputeAngles(fullAngle, startAngle, endAngle);
      return fullAngle;
    }

    void SetStart(const ScenePoint2D& p)
    {
      SetModified(true);
      start_ = p;
    }

    void SetMiddle(const ScenePoint2D& p)
    {
      SetModified(true);
      middle_ = p;
    }

    void SetEnd(const ScenePoint2D& p)
    {
      SetModified(true);
      end_ = p;
    }

    virtual bool IsHit(const ScenePoint2D& p,
                       const Scene2D& scene) const ORTHANC_OVERRIDE
    {
      return false;
    }

    virtual void RenderPolylineLayer(PolylineSceneLayer& polyline,
                                     const Scene2D& scene) ORTHANC_OVERRIDE
    {
      static unsigned int NUM_SEGMENTS = 64;

      const double radius = radius_ / scene.GetSceneToCanvasTransform().ComputeZoom();

      double fullAngle, startAngle, endAngle;
      ComputeAngles(fullAngle, startAngle, endAngle);

      double increment = fullAngle / static_cast<double>(NUM_SEGMENTS - 1);

      PolylineSceneLayer::Chain chain;
      chain.reserve(NUM_SEGMENTS);

      double theta = startAngle;
      for (unsigned int i = 0; i < NUM_SEGMENTS; i++)
      {
        chain.push_back(ScenePoint2D(middle_.GetX() + radius * cos(theta),
                                     middle_.GetY() + radius * sin(theta)));
        theta += increment;
      }
        
      if (IsHover())
      {
        polyline.AddChain(chain, false /* closed */, GetHoverColor());
      }
      else
      {
        polyline.AddChain(chain, false /* closed */, GetColor());
      }
    }
      
    virtual void RenderOtherLayers(MacroSceneLayer& macro,
                                   const Scene2D& scene) ORTHANC_OVERRIDE
    {
    }

    virtual void MovePreview(const ScenePoint2D& delta) ORTHANC_OVERRIDE
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);  // No hit is possible
    }

    virtual void MoveDone(const ScenePoint2D& delta) ORTHANC_OVERRIDE
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);  // No hit is possible
    }
  };

    
  class AnnotationsSceneLayer::Text : public GeometricPrimitive
  {
  private:
    AnnotationsSceneLayer&           that_;
    bool                             first_;
    size_t                           subLayer_;
    std::unique_ptr<TextSceneLayer>  content_;

  public:
    Text(AnnotationsSceneLayer& that,
         Annotation& parentAnnotation) :
      GeometricPrimitive(parentAnnotation, 2),
      that_(that),
      first_(true),
      subLayer_(0)  // dummy initialization
    {
    }

    virtual ~Text()
    {
      if (!first_)
      {
        that_.TagSubLayerToRemove(subLayer_);
      }
    }
      
    void SetContent(const TextSceneLayer& content)
    {
      SetModified(true);
      content_.reset(dynamic_cast<TextSceneLayer*>(content.Clone()));
    }        

    virtual bool IsHit(const ScenePoint2D& p,
                       const Scene2D& scene) const ORTHANC_OVERRIDE
    {
      return false;
    }

    virtual void RenderPolylineLayer(PolylineSceneLayer& polyline,
                                     const Scene2D& scene) ORTHANC_OVERRIDE
    {
    }
      
    virtual void RenderOtherLayers(MacroSceneLayer& macro,
                                   const Scene2D& scene) ORTHANC_OVERRIDE
    {
      if (content_.get() != NULL)
      {
        std::unique_ptr<TextSceneLayer> layer(reinterpret_cast<TextSceneLayer*>(content_->Clone()));

        layer->SetColor(IsHover() ? GetHoverColor() : GetColor());
          
        if (first_)
        {
          subLayer_ = macro.AddLayer(layer.release());
          first_ = false;
        }
        else
        {
          macro.UpdateLayer(subLayer_, layer.release());
        }
      }
    }

    virtual void MovePreview(const ScenePoint2D& delta) ORTHANC_OVERRIDE
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);  // No hit is possible
    }

    virtual void MoveDone(const ScenePoint2D& delta) ORTHANC_OVERRIDE
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);  // No hit is possible
    }
  };


  class AnnotationsSceneLayer::EditPrimitiveTracker : public IFlexiblePointerTracker
  {
  private:
    AnnotationsSceneLayer&  that_;
    GeometricPrimitive&     primitive_;
    ScenePoint2D            sceneClick_;
    AffineTransform2D       canvasToScene_;
    bool                    alive_;
      
  public:
    EditPrimitiveTracker(AnnotationsSceneLayer& that,
                         GeometricPrimitive& primitive,
                         const ScenePoint2D& sceneClick,
                         const AffineTransform2D& canvasToScene) :
      that_(that),
      primitive_(primitive),
      sceneClick_(sceneClick),
      canvasToScene_(canvasToScene),
      alive_(true)
    {
    }

    virtual void PointerMove(const PointerEvent& event) ORTHANC_OVERRIDE
    {
      primitive_.MovePreview(event.GetMainPosition().Apply(canvasToScene_) - sceneClick_);
      that_.BroadcastMessage(AnnotationChangedMessage(that_));
    }
      
    virtual void PointerUp(const PointerEvent& event) ORTHANC_OVERRIDE
    {
      primitive_.MoveDone(event.GetMainPosition().Apply(canvasToScene_) - sceneClick_);
      alive_ = false;
      that_.BroadcastMessage(AnnotationChangedMessage(that_));
    }

    virtual void PointerDown(const PointerEvent& event) ORTHANC_OVERRIDE
    {
    }

    virtual bool IsAlive() const ORTHANC_OVERRIDE
    {
      return alive_;
    }

    virtual void Cancel() ORTHANC_OVERRIDE
    {
      primitive_.MoveDone(ScenePoint2D(0, 0));
    }
  };


  class AnnotationsSceneLayer::SegmentAnnotation : public Annotation
  {
  private:
    bool      showLabel_;
    Handle&   handle1_;
    Handle&   handle2_;
    Segment&  segment_;
    Text&     label_;

    void UpdateLabel()
    {
      if (showLabel_)
      {
        TextSceneLayer content;

        double x1 = handle1_.GetCenter().GetX();
        double y1 = handle1_.GetCenter().GetY();
        double x2 = handle2_.GetCenter().GetX();
        double y2 = handle2_.GetCenter().GetY();
        
        // Put the label to the right of the right-most handle
        if (x1 < x2)
        {
          content.SetPosition(x2, y2);
        }
        else
        {
          content.SetPosition(x1, y1);
        }

        content.SetAnchor(BitmapAnchor_CenterLeft);
        content.SetBorder(10);

        double dx = x1 - x2;
        double dy = y1 - y2;
        char buf[32];

        switch (GetUnits())
        {
          case Units_Millimeters:
            sprintf(buf, "%0.2f cm", sqrt(dx * dx + dy * dy) / 10.0);
            break;

          case Units_Pixels:
            sprintf(buf, "%0.1f px", sqrt(dx * dx + dy * dy));
            break;

          default:
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }
            
        content.SetText(buf);

        label_.SetContent(content);
      }
    }

  public:
    SegmentAnnotation(AnnotationsSceneLayer& that,
                      Units units,
                      bool showLabel,
                      const ScenePoint2D& p1,
                      const ScenePoint2D& p2) :
      Annotation(that, units),
      showLabel_(showLabel),
      handle1_(AddTypedPrimitive<Handle>(new Handle(*this, p1))),
      handle2_(AddTypedPrimitive<Handle>(new Handle(*this, p2))),
      segment_(AddTypedPrimitive<Segment>(new Segment(*this, p1, p2))),
      label_(AddTypedPrimitive<Text>(new Text(that, *this)))
    {
      label_.SetColor(COLOR_TEXT);
      UpdateLabel();
    }

    Handle& GetHandle1() const
    {
      return handle1_;
    }

    Handle& GetHandle2() const
    {
      return handle2_;
    }

    virtual void SignalMove(GeometricPrimitive& primitive) ORTHANC_OVERRIDE
    {
      if (&primitive == &handle1_ ||
          &primitive == &handle2_)
      {
        segment_.SetPosition(handle1_.GetCenter(), handle2_.GetCenter());
      }
      else if (&primitive == &segment_)
      {
        handle1_.SetCenter(segment_.GetPosition1());
        handle2_.SetCenter(segment_.GetPosition2());
      }
        
      UpdateLabel();
    }

    virtual void Serialize(Json::Value& target) ORTHANC_OVERRIDE
    {
      target = Json::objectValue;
      target[KEY_TYPE] = VALUE_SEGMENT;
      target[KEY_X1] = handle1_.GetCenter().GetX();
      target[KEY_Y1] = handle1_.GetCenter().GetY();
      target[KEY_X2] = handle2_.GetCenter().GetX();
      target[KEY_Y2] = handle2_.GetCenter().GetY();
    }

    static void Unserialize(AnnotationsSceneLayer& target,
                            Units units,
                            const Json::Value& source)
    {
      if (source.isMember(KEY_X1) &&
          source.isMember(KEY_Y1) &&
          source.isMember(KEY_X2) &&
          source.isMember(KEY_Y2) &&
          source[KEY_X1].isNumeric() &&
          source[KEY_Y1].isNumeric() &&
          source[KEY_X2].isNumeric() &&
          source[KEY_Y2].isNumeric())
      {
        new SegmentAnnotation(target, units, true,
                              ScenePoint2D(source[KEY_X1].asDouble(), source[KEY_Y1].asDouble()),
                              ScenePoint2D(source[KEY_X2].asDouble(), source[KEY_Y2].asDouble()));
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat, "Cannot unserialize an segment annotation");
      }
    }
  };

    
  class AnnotationsSceneLayer::AngleAnnotation : public Annotation
  {
  private:
    Handle&   startHandle_;
    Handle&   middleHandle_;
    Handle&   endHandle_;
    Segment&  segment1_;
    Segment&  segment2_;
    Arc&      arc_;
    Text&     label_;

    void UpdateLabel()
    {
      TextSceneLayer content;

      const double x1 = startHandle_.GetCenter().GetX();
      const double x2 = middleHandle_.GetCenter().GetX();
      const double y2 = middleHandle_.GetCenter().GetY();
      const double x3 = endHandle_.GetCenter().GetX();
        
      if (x2 < x1 &&
          x2 < x3)
      {
        content.SetAnchor(BitmapAnchor_CenterRight);
      }
      else
      {
        content.SetAnchor(BitmapAnchor_CenterLeft);
      }

      content.SetPosition(x2, y2);
      content.SetBorder(10);

      char buf[32];
      sprintf(buf, "%.01f%c%c", std::abs(arc_.GetAngle()) / PI * 180.0,
              0xc2, 0xb0 /* two bytes corresponding to degree symbol in UTF-8 */);
      content.SetText(buf);

      label_.SetContent(content);
    }

  public:
    AngleAnnotation(AnnotationsSceneLayer& that,
                    Units units,
                    const ScenePoint2D& start,
                    const ScenePoint2D& middle,
                    const ScenePoint2D& end) :
      Annotation(that, units),
      startHandle_(AddTypedPrimitive<Handle>(new Handle(*this, start))),
      middleHandle_(AddTypedPrimitive<Handle>(new Handle(*this, middle))),
      endHandle_(AddTypedPrimitive<Handle>(new Handle(*this, end))),
      segment1_(AddTypedPrimitive<Segment>(new Segment(*this, start, middle))),
      segment2_(AddTypedPrimitive<Segment>(new Segment(*this, middle, end))),
      arc_(AddTypedPrimitive<Arc>(new Arc(*this, start, middle, end))),
      label_(AddTypedPrimitive<Text>(new Text(that, *this)))
    {
      label_.SetColor(COLOR_TEXT);
      UpdateLabel();
    }

    Handle& GetEndHandle() const
    {
      return endHandle_;
    }

    virtual void SignalMove(GeometricPrimitive& primitive) ORTHANC_OVERRIDE
    {
      if (&primitive == &startHandle_)
      {
        segment1_.SetPosition(startHandle_.GetCenter(), middleHandle_.GetCenter());
        arc_.SetStart(startHandle_.GetCenter());
      }
      else if (&primitive == &middleHandle_)
      {
        segment1_.SetPosition(startHandle_.GetCenter(), middleHandle_.GetCenter());
        segment2_.SetPosition(middleHandle_.GetCenter(), endHandle_.GetCenter());
        arc_.SetMiddle(middleHandle_.GetCenter());
      }
      else if (&primitive == &endHandle_)
      {
        segment2_.SetPosition(middleHandle_.GetCenter(), endHandle_.GetCenter());
        arc_.SetEnd(endHandle_.GetCenter());
      }
      else if (&primitive == &segment1_)
      {
        startHandle_.SetCenter(segment1_.GetPosition1());
        middleHandle_.SetCenter(segment1_.GetPosition2());
        segment2_.SetPosition(segment1_.GetPosition2(), segment2_.GetPosition2());
        arc_.SetStart(segment1_.GetPosition1());
        arc_.SetMiddle(segment1_.GetPosition2());
      }
      else if (&primitive == &segment2_)
      {
        middleHandle_.SetCenter(segment2_.GetPosition1());
        endHandle_.SetCenter(segment2_.GetPosition2());
        segment1_.SetPosition(segment1_.GetPosition1(), segment2_.GetPosition1());
        arc_.SetMiddle(segment2_.GetPosition1());
        arc_.SetEnd(segment2_.GetPosition2());
      }

      UpdateLabel();
    }

    virtual void Serialize(Json::Value& target) ORTHANC_OVERRIDE
    {
      target = Json::objectValue;
      target[KEY_TYPE] = VALUE_ANGLE;
      target[KEY_X1] = startHandle_.GetCenter().GetX();
      target[KEY_Y1] = startHandle_.GetCenter().GetY();
      target[KEY_X2] = middleHandle_.GetCenter().GetX();
      target[KEY_Y2] = middleHandle_.GetCenter().GetY();
      target[KEY_X3] = endHandle_.GetCenter().GetX();
      target[KEY_Y3] = endHandle_.GetCenter().GetY();
    }

    static void Unserialize(AnnotationsSceneLayer& target,
                            Units units,
                            const Json::Value& source)
    {
      if (source.isMember(KEY_X1) &&
          source.isMember(KEY_Y1) &&
          source.isMember(KEY_X2) &&
          source.isMember(KEY_Y2) &&
          source.isMember(KEY_X3) &&
          source.isMember(KEY_Y3) &&
          source[KEY_X1].isNumeric() &&
          source[KEY_Y1].isNumeric() &&
          source[KEY_X2].isNumeric() &&
          source[KEY_Y2].isNumeric() &&
          source[KEY_X3].isNumeric() &&
          source[KEY_Y3].isNumeric())
      {
        new AngleAnnotation(target, units,
                            ScenePoint2D(source[KEY_X1].asDouble(), source[KEY_Y1].asDouble()),
                            ScenePoint2D(source[KEY_X2].asDouble(), source[KEY_Y2].asDouble()),
                            ScenePoint2D(source[KEY_X3].asDouble(), source[KEY_Y3].asDouble()));
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat, "Cannot unserialize an angle annotation");
      }
    }
  };

    
  class AnnotationsSceneLayer::CircleAnnotation : public Annotation
  {
  private:
    Handle&   handle1_;
    Handle&   handle2_;
    Segment&  segment_;
    Circle&   circle_;
    Text&     label_;

    void UpdateLabel()
    {
      TextSceneLayer content;

      double x1 = handle1_.GetCenter().GetX();
      double y1 = handle1_.GetCenter().GetY();
      double x2 = handle2_.GetCenter().GetX();
      double y2 = handle2_.GetCenter().GetY();
        
      // Put the label to the right of the right-most handle
      if (x1 < x2)
      {
        content.SetPosition(x2, y2);
      }
      else
      {
        content.SetPosition(x1, y1);
      }

      content.SetAnchor(BitmapAnchor_CenterLeft);
      content.SetBorder(10);

      double dx = x1 - x2;
      double dy = y1 - y2;
      double diameter = sqrt(dx * dx + dy * dy);  // in millimeters

      double area = PI * diameter * diameter / 4.0;
        
      char buf[32];

      switch (GetUnits())
      {
        case Units_Millimeters:
          sprintf(buf, "%0.2f cm\n%0.2f cm%c%c",
                  diameter / 10.0,
                  area / 100.0,
                  0xc2, 0xb2 /* two bytes corresponding to two power in UTF-8 */);
          break;

        case Units_Pixels:
          // Don't report area (pixel-times-pixel is a strange unit)
          sprintf(buf, "%0.1f px", diameter);
          break;
          
        default:
          throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
      }

      content.SetText(buf);

      label_.SetContent(content);
    }

  public:
    CircleAnnotation(AnnotationsSceneLayer& that,
                     Units units,
                     const ScenePoint2D& p1,
                     const ScenePoint2D& p2) :
      Annotation(that, units),
      handle1_(AddTypedPrimitive<Handle>(new Handle(*this, p1))),
      handle2_(AddTypedPrimitive<Handle>(new Handle(*this, p2))),
      segment_(AddTypedPrimitive<Segment>(new Segment(*this, p1, p2))),
      circle_(AddTypedPrimitive<Circle>(new Circle(*this, p1, p2))),
      label_(AddTypedPrimitive<Text>(new Text(that, *this)))
    {
      label_.SetColor(COLOR_TEXT);
      UpdateLabel();
    }

    Handle& GetHandle2() const
    {
      return handle2_;
    }

    virtual void SignalMove(GeometricPrimitive& primitive) ORTHANC_OVERRIDE
    {
      if (&primitive == &handle1_ ||
          &primitive == &handle2_)
      {
        segment_.SetPosition(handle1_.GetCenter(), handle2_.GetCenter());
        circle_.SetPosition(handle1_.GetCenter(), handle2_.GetCenter());          
      }
      else if (&primitive == &segment_)
      {
        handle1_.SetCenter(segment_.GetPosition1());
        handle2_.SetCenter(segment_.GetPosition2());
        circle_.SetPosition(segment_.GetPosition1(), segment_.GetPosition2());
      }
      else if (&primitive == &circle_)
      {
        handle1_.SetCenter(circle_.GetPosition1());
        handle2_.SetCenter(circle_.GetPosition2());
        segment_.SetPosition(circle_.GetPosition1(), circle_.GetPosition2());
      }
        
      UpdateLabel();
    }

    virtual void Serialize(Json::Value& target) ORTHANC_OVERRIDE
    {
      target = Json::objectValue;
      target[KEY_TYPE] = VALUE_CIRCLE;
      target[KEY_X1] = handle1_.GetCenter().GetX();
      target[KEY_Y1] = handle1_.GetCenter().GetY();
      target[KEY_X2] = handle2_.GetCenter().GetX();
      target[KEY_Y2] = handle2_.GetCenter().GetY();
    }

    static void Unserialize(AnnotationsSceneLayer& target,
                            Units units,
                            const Json::Value& source)
    {
      if (source.isMember(KEY_X1) &&
          source.isMember(KEY_Y1) &&
          source.isMember(KEY_X2) &&
          source.isMember(KEY_Y2) &&
          source[KEY_X1].isNumeric() &&
          source[KEY_Y1].isNumeric() &&
          source[KEY_X2].isNumeric() &&
          source[KEY_Y2].isNumeric())
      {
        new CircleAnnotation(target, units,
                             ScenePoint2D(source[KEY_X1].asDouble(), source[KEY_Y1].asDouble()),
                             ScenePoint2D(source[KEY_X2].asDouble(), source[KEY_Y2].asDouble()));
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat, "Cannot unserialize an circle annotation");
      }
    }
  };

    
  class AnnotationsSceneLayer::CreateSegmentOrCircleTracker : public IFlexiblePointerTracker
  {
  private:
    AnnotationsSceneLayer&  that_;
    Annotation*             annotation_;
    AffineTransform2D       canvasToScene_;
    Handle*                 handle2_;
      
  public:
    CreateSegmentOrCircleTracker(AnnotationsSceneLayer& that,
                                 Units units,
                                 bool isCircle,
                                 const ScenePoint2D& sceneClick,
                                 const AffineTransform2D& canvasToScene) :
      that_(that),
      annotation_(NULL),
      canvasToScene_(canvasToScene),
      handle2_(NULL)
    {
      if (isCircle)
      {
        annotation_ = new CircleAnnotation(that, units, sceneClick, sceneClick);
        handle2_ = &dynamic_cast<CircleAnnotation*>(annotation_)->GetHandle2();
      }
      else
      {
        annotation_ = new SegmentAnnotation(that, units, true /* show label */, sceneClick, sceneClick);
        handle2_ = &dynamic_cast<SegmentAnnotation*>(annotation_)->GetHandle2();
      }
        
      assert(annotation_ != NULL &&
             handle2_ != NULL);
    }

    virtual void PointerMove(const PointerEvent& event) ORTHANC_OVERRIDE
    {
      if (annotation_ != NULL)
      {
        assert(handle2_ != NULL);
        handle2_->SetCenter(event.GetMainPosition().Apply(canvasToScene_));
        annotation_->SignalMove(*handle2_);

        that_.BroadcastMessage(AnnotationChangedMessage(that_));
      }
    }
      
    virtual void PointerUp(const PointerEvent& event) ORTHANC_OVERRIDE
    {
      annotation_ = NULL;  // IsAlive() becomes false

      that_.BroadcastMessage(AnnotationAddedMessage(that_));
    }

    virtual void PointerDown(const PointerEvent& event) ORTHANC_OVERRIDE
    {
    }

    virtual bool IsAlive() const ORTHANC_OVERRIDE
    {
      return (annotation_ != NULL);
    }

    virtual void Cancel() ORTHANC_OVERRIDE
    {
      if (annotation_ != NULL)
      {
        that_.DeleteAnnotation(annotation_);
        annotation_ = NULL;
      }
    }
  };


  class AnnotationsSceneLayer::CreateAngleTracker : public IFlexiblePointerTracker
  {
  private:
    AnnotationsSceneLayer&  that_;
    SegmentAnnotation*      segment_;
    AngleAnnotation*        angle_;
    AffineTransform2D       canvasToScene_;
      
  public:
    CreateAngleTracker(AnnotationsSceneLayer& that,
                       Units units,
                       const ScenePoint2D& sceneClick,
                       const AffineTransform2D& canvasToScene) :
      that_(that),
      segment_(NULL),
      angle_(NULL),
      canvasToScene_(canvasToScene)
    {
      segment_ = new SegmentAnnotation(that, units, false /* no length label */, sceneClick, sceneClick);
    }

    virtual void PointerMove(const PointerEvent& event) ORTHANC_OVERRIDE
    {
      if (segment_ != NULL)
      {
        segment_->GetHandle2().SetCenter(event.GetMainPosition().Apply(canvasToScene_));
        segment_->SignalMove(segment_->GetHandle2());
        that_.BroadcastMessage(AnnotationChangedMessage(that_));
      }

      if (angle_ != NULL)
      {
        angle_->GetEndHandle().SetCenter(event.GetMainPosition().Apply(canvasToScene_));
        angle_->SignalMove(angle_->GetEndHandle());
        that_.BroadcastMessage(AnnotationChangedMessage(that_));
      }
    }
      
    virtual void PointerUp(const PointerEvent& event) ORTHANC_OVERRIDE
    {
      if (segment_ != NULL)
      {
        // End of first step: The first segment is available, now create the angle

        angle_ = new AngleAnnotation(that_, segment_->GetUnits(), segment_->GetHandle1().GetCenter(),
                                     segment_->GetHandle2().GetCenter(),
                                     segment_->GetHandle2().GetCenter());
          
        that_.DeleteAnnotation(segment_);
        segment_ = NULL;

        that_.BroadcastMessage(AnnotationChangedMessage(that_));
      }
      else
      {
        angle_ = NULL;  // IsAlive() becomes false

        that_.BroadcastMessage(AnnotationAddedMessage(that_));
      }
    }

    virtual void PointerDown(const PointerEvent& event) ORTHANC_OVERRIDE
    {
    }

    virtual bool IsAlive() const ORTHANC_OVERRIDE
    {
      return (segment_ != NULL ||
              angle_ != NULL);
    }

    virtual void Cancel() ORTHANC_OVERRIDE
    {
      if (segment_ != NULL)
      {
        that_.DeleteAnnotation(segment_);
        segment_ = NULL;
      }

      if (angle_ != NULL)
      {
        that_.DeleteAnnotation(angle_);
        angle_ = NULL;
      }
    }
  };


  // Dummy tracker that is only used for deletion, in order to warn
  // the caller that the mouse action was taken into consideration
  class AnnotationsSceneLayer::RemoveTracker : public IFlexiblePointerTracker
  {
  public:
    RemoveTracker()
    {
    }

    virtual void PointerMove(const PointerEvent& event) ORTHANC_OVERRIDE
    {
    }
      
    virtual void PointerUp(const PointerEvent& event) ORTHANC_OVERRIDE
    {
    }

    virtual void PointerDown(const PointerEvent& event) ORTHANC_OVERRIDE
    {
    }

    virtual bool IsAlive() const ORTHANC_OVERRIDE
    {
      return false;
    }

    virtual void Cancel() ORTHANC_OVERRIDE
    {
    }
  };


  void AnnotationsSceneLayer::AddAnnotation(Annotation* annotation)
  {
    assert(annotation != NULL);
    assert(annotations_.find(annotation) == annotations_.end());
    annotations_.insert(annotation);
  }
  

  void AnnotationsSceneLayer::DeleteAnnotation(Annotation* annotation)
  {
    if (annotation != NULL)
    {
      assert(annotations_.find(annotation) != annotations_.end());
      annotations_.erase(annotation);
      delete annotation;
    }
  }

  
  void AnnotationsSceneLayer::DeletePrimitive(GeometricPrimitive* primitive)
  {
    if (primitive != NULL)
    {
      assert(primitives_.find(primitive) != primitives_.end());
      primitives_.erase(primitive);
      delete primitive;
    }
  }

  
  void AnnotationsSceneLayer::TagSubLayerToRemove(size_t subLayerIndex)
  {
    assert(subLayersToRemove_.find(subLayerIndex) == subLayersToRemove_.end());
    subLayersToRemove_.insert(subLayerIndex);
  }
    

  AnnotationsSceneLayer::AnnotationsSceneLayer(size_t macroLayerIndex) :
    activeTool_(Tool_Edit),
    macroLayerIndex_(macroLayerIndex),
    polylineSubLayer_(0),  // dummy initialization
    units_(Units_Pixels)
  {
  }
    

  void AnnotationsSceneLayer::Clear()
  {
    for (Annotations::iterator it = annotations_.begin(); it != annotations_.end(); ++it)
    {
      assert(*it != NULL);
      delete *it;
    }

    annotations_.clear();

    ClearHover();
  }

  
  void AnnotationsSceneLayer::SetUnits(Units units)
  {
    if (units_ != units)
    {
      Clear();
      units_ = units;
    }
  }


  void AnnotationsSceneLayer::AddSegmentAnnotation(const ScenePoint2D& p1,
                                                   const ScenePoint2D& p2)
  {
    annotations_.insert(new SegmentAnnotation(*this, units_, true /* show label */, p1, p2));
  }
  

  void AnnotationsSceneLayer::AddCircleAnnotation(const ScenePoint2D& p1,
                                                  const ScenePoint2D& p2)
  {
    annotations_.insert(new CircleAnnotation(*this, units_, p1, p2));
  }
  

  void AnnotationsSceneLayer::AddAngleAnnotation(const ScenePoint2D& p1,
                                                 const ScenePoint2D& p2,
                                                 const ScenePoint2D& p3)
  {
    annotations_.insert(new AngleAnnotation(*this, units_, p1, p2, p3));
  }
  

  void AnnotationsSceneLayer::Render(Scene2D& scene)
  {
    MacroSceneLayer* macro = NULL;

    if (scene.HasLayer(macroLayerIndex_))
    {
      macro = &dynamic_cast<MacroSceneLayer&>(scene.GetLayer(macroLayerIndex_));
    }
    else
    {
      macro = &dynamic_cast<MacroSceneLayer&>(scene.SetLayer(macroLayerIndex_, new MacroSceneLayer));
      polylineSubLayer_ = macro->AddLayer(new PolylineSceneLayer);
    }

    for (SubLayers::const_iterator it = subLayersToRemove_.begin(); it != subLayersToRemove_.end(); ++it)
    {
      assert(macro->HasLayer(*it));
      macro->DeleteLayer(*it);
    }

    subLayersToRemove_.clear();

    std::unique_ptr<PolylineSceneLayer> polyline(new PolylineSceneLayer);

    for (GeometricPrimitives::iterator it = primitives_.begin(); it != primitives_.end(); ++it)
    {
      assert(*it != NULL);
      GeometricPrimitive& primitive = **it;        
        
      primitive.RenderPolylineLayer(*polyline, scene);

      if (primitive.IsModified())
      {
        primitive.RenderOtherLayers(*macro, scene);
        primitive.SetModified(false);
      }
    }

    macro->UpdateLayer(polylineSubLayer_, polyline.release());
  }

  
  bool AnnotationsSceneLayer::ClearHover()
  {
    bool needsRefresh = false;
      
    for (GeometricPrimitives::iterator it = primitives_.begin(); it != primitives_.end(); ++it)
    {
      assert(*it != NULL);
      if ((*it)->IsHover())
      {
        (*it)->SetHover(false);
        needsRefresh = true;
      }
    }

    return needsRefresh;
  }
  

  bool AnnotationsSceneLayer::SetMouseHover(const ScenePoint2D& p,
                                            const Scene2D& scene)
  {
    if (activeTool_ == Tool_None)
    {
      return ClearHover();
    }
    else
    {
      bool needsRefresh = false;
      
      const ScenePoint2D s = p.Apply(scene.GetCanvasToSceneTransform());
      
      for (GeometricPrimitives::iterator it = primitives_.begin(); it != primitives_.end(); ++it)
      {
        assert(*it != NULL);
        bool hover = (*it)->IsHit(s, scene);

        if ((*it)->IsHover() != hover)
        {
          needsRefresh = true;
        }
        
        (*it)->SetHover(hover);
      }

      return needsRefresh;
    }
  }


  IFlexiblePointerTracker* AnnotationsSceneLayer::CreateTracker(const ScenePoint2D& p,
                                                                const Scene2D& scene)
  {
    if (activeTool_ == Tool_None)
    {
      return NULL;
    }
    else
    {
      const ScenePoint2D s = p.Apply(scene.GetCanvasToSceneTransform());

      GeometricPrimitive* bestHit = NULL;
      
      for (GeometricPrimitives::iterator it = primitives_.begin(); it != primitives_.end(); ++it)
      {
        assert(*it != NULL);
        if ((*it)->IsHit(s, scene))
        {
          if (bestHit == NULL ||
              bestHit->GetDepth() > (*it)->GetDepth())
          {
            bestHit = *it;
          }
        }
      }

      if (bestHit != NULL)
      {
        if (activeTool_ == Tool_Remove)
        {
          DeleteAnnotation(&bestHit->GetParentAnnotation());
          BroadcastMessage(AnnotationRemovedMessage(*this));
          return new RemoveTracker;
        }
        else
        {
          return new EditPrimitiveTracker(*this, *bestHit, s, scene.GetCanvasToSceneTransform());
        }
      }
      else
      {
        switch (activeTool_)
        {
          case Tool_Segment:
            return new CreateSegmentOrCircleTracker(*this, units_, false /* segment */, s, scene.GetCanvasToSceneTransform());

          case Tool_Circle:
            return new CreateSegmentOrCircleTracker(*this, units_, true /* circle */, s, scene.GetCanvasToSceneTransform());

          case Tool_Angle:
            return new CreateAngleTracker(*this, units_, s, scene.GetCanvasToSceneTransform());

          default:
            return NULL;
        }
      }
    }
  }


  void AnnotationsSceneLayer::Serialize(Json::Value& target) const
  {
    Json::Value annotations = Json::arrayValue;
      
    for (Annotations::const_iterator it = annotations_.begin(); it != annotations_.end(); ++it)
    {
      assert(*it != NULL);

      Json::Value item;
      (*it)->Serialize(item);
      annotations.append(item);
    }

    target = Json::objectValue;
    target[KEY_ANNOTATIONS] = annotations;

    switch (units_)
    {
      case Units_Millimeters:
        target[KEY_UNITS] = VALUE_MILLIMETERS;
        break;

      case Units_Pixels:
        target[KEY_UNITS] = VALUE_PIXELS;
        break;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }


  void AnnotationsSceneLayer::Unserialize(const Json::Value& serialized)
  {
    Clear();
      
    if (serialized.type() != Json::objectValue ||
        !serialized.isMember(KEY_ANNOTATIONS) ||
        !serialized.isMember(KEY_UNITS) ||
        serialized[KEY_ANNOTATIONS].type() != Json::arrayValue ||
        serialized[KEY_UNITS].type() != Json::stringValue)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat, "Cannot unserialize a set of annotations");
    }

    const std::string& u = serialized[KEY_UNITS].asString();

    if (u == VALUE_MILLIMETERS)
    {
      units_ = Units_Millimeters;
    }
    else if (u == VALUE_PIXELS)
    {
      units_ = Units_Pixels;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat, "Unknown units: " + u);
    }

    const Json::Value& annotations = serialized[KEY_ANNOTATIONS];

    for (Json::Value::ArrayIndex i = 0; i < annotations.size(); i++)
    {
      if (annotations[i].type() != Json::objectValue ||
          !annotations[i].isMember(KEY_TYPE) ||
          annotations[i][KEY_TYPE].type() != Json::stringValue)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }

      const std::string& type = annotations[i][KEY_TYPE].asString();

      if (type == VALUE_ANGLE)
      {
        AngleAnnotation::Unserialize(*this, units_, annotations[i]);
      }
      else if (type == VALUE_CIRCLE)
      {
        CircleAnnotation::Unserialize(*this, units_, annotations[i]);
      }
      else if (type == VALUE_SEGMENT)
      {
        SegmentAnnotation::Unserialize(*this, units_, annotations[i]);
      }
      else
      {
        LOG(ERROR) << "Cannot unserialize unknown type of annotation: " << type;
      }
    }
  }
}
