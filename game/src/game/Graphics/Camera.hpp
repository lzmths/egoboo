//********************************************************************************************
//*
//*    This file is part of Egoboo.
//*
//*    Egoboo is free software: you can redistribute it and/or modify it
//*    under the terms of the GNU General Public License as published by
//*    the Free Software Foundation, either version 3 of the License, or
//*    (at your option) any later version.
//*
//*    Egoboo is distributed in the hope that it will be useful, but
//*    WITHOUT ANY WARRANTY; without even the implied warranty of
//*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//*    General Public License for more details.
//*
//*    You should have received a copy of the GNU General Public License
//*    along with Egoboo.  If not, see <http://www.gnu.org/licenses/>.
//*
//********************************************************************************************
/// @file game/Graphics/Camera.hpp
/// @author Johan Jansen

#pragma once

#include "game/egoboo_typedef.h"
#include "game/physics.h"			//for orientation_t
#include "egolib/Graphics/Camera.hpp"

//Forward declarations
class ego_mesh_t;
namespace Ego {
namespace Graphics {
struct TileList;
struct EntityList;
}
}

/// The mode that the camera uses to determine where it is moving.
enum class CameraMovementMode : uint8_t
{
    /// Camera movement mode for (re-)focusing in on one or more player objects doing something.
    /// "Show me the drama!"
    Player,
    /// Camera movement mode for controlling the camera movement by keybpad.
    Free,
    /// Camera movement mode for (re-)focusing in on one or more objects.
    Reset,
};

/// The mode that the camera uses to determine where is is looking.
enum class CameraTurnMode : uint8_t
{
    None = 0,
    Auto = 1,
    Good = 255
};

struct CameraOptions
{
    int            swing;                   ///< Camera swing angle
    int            swingRate;               ///< Camera swing rate
    float          swingAmp;                ///< Camera swing amplitude
	CameraTurnMode turnMode;                ///< what is the camera turn mode
};

/// Multi cam uses macro to switch between old and new camera
static constexpr float CAM_ZOOM_FACTOR = 0.5f;
#ifdef OLD_CAMERA_MODE
    static constexpr float CAM_ZOOM_MIN = (800 * CAM_ZOOM_FACTOR);       ///< Camera distance
    static constexpr float CAM_ZOOM_MAX = (700 * CAM_ZOOM_FACTOR);
    static constexpr float CAM_ZADD_MIN = (800 * CAM_ZOOM_FACTOR);       ///< Camera height
    static constexpr float CAM_ZADD_MAX = (2750 * CAM_ZOOM_FACTOR);
#else
    static constexpr float CAM_ZOOM_MIN = (500 * CAM_ZOOM_FACTOR);       ///< Camera distance
    static constexpr float CAM_ZOOM_MAX = (800 * CAM_ZOOM_FACTOR);
    static constexpr float CAM_ZADD_MIN = (800 * CAM_ZOOM_FACTOR);       ///< Camera height
    static constexpr float CAM_ZADD_MAX = (1900 * CAM_ZOOM_FACTOR);
#endif

/**
 * @brief
 *  Egoboo's camera.
 * @remark
 *  - "center" the position the camera is focused on
 *  - "position" the position the camera is located at
 */
class Camera : public Id::NonCopyable, public Ego::Graphics::Camera
{

public:

	/** @name "Implementation of Ego::Graphics::Camera" */
	/**@{*/

private:

	/**
	 * @brief
	 *  The forward vector of this camera.
	 */
	Vector3f _forward;

	/**
	 * @brief
	 *  The up vector of this camera.
	 */
	Vector3f _up;

	/**
	 * @brief
	 *  The right vector of this camera.
	 */
	Vector3f _right;

	/**
	 * @brief
	 *  The view matrix (derived/cached from other attributes).
	 */
	Matrix4f4f _viewMatrix;

	/**
	 * @brief
	 *  The projection matrices (derived/cached from other attributes).
	 */
	Matrix4f4f _projectionMatrix;

	/**
	 * @brief
	 *  The position.
	 * @invariant
	 *  @a z must be within the interval <tt>[500,1000]</tt>.
	 */
	Vector3f _position;

public:

	/** @copydoc ICamera::getProjectionMatrix */
	inline const Matrix4f4f& getProjectionMatrix() const override { return _projectionMatrix; }

	/** @copydoc ICamera::getViewMatrix */
	inline const Matrix4f4f& getViewMatrix() const override { return _viewMatrix; }

	/** @copydoc ICamera::getPosition */
	inline const Vector3f& getPosition() const override { return _position; }

	/** @copydoc ICamera::getUp */
	inline const Vector3f& getUp() const override { return _up; }
	
	/** @copydoc ICamera::getRight */
	inline const Vector3f& getRight() const override { return _right; }
	
	/** @copydoc ICamera::getForward */
	inline const Vector3f& getForward() const override { return _forward; }

	/**@}*/

public:
	/**
	 * @brief
	 *  Construct this camera.
	 * @param options
	 *  the camera options
	 */
    Camera(const CameraOptions &options);
	/**
	 * @brief
	 *  Destruct this camera.
	 */
    virtual ~Camera();

    /// The default field of view angle (in degrees).
    static const float DEFAULT_FOV;

    /// The default joystick turn rotation.
    /// @todo What unit is that?
    static const float DEFAULT_TURN_JOY;

    /// The default keyboard turn rotation.
    /// @todo What unit is that?
    static const float DEFAULT_TURN_KEY;

    /// The default smooth turn rotation.
    /// @todo What unit is that?
    static const uint8_t DEFAULT_TURN_TIME;

    static const float CAM_ZADD_AVG;
    static const float CAM_ZOOM_AVG;

    /**
     * @brief
     *  Initialization that has to be after object construction.
     */
    void initialize(std::shared_ptr<Ego::Graphics::TileList> tileList, std::shared_ptr<Ego::Graphics::EntityList> entityList);

    // various getters
    inline const Ego::Graphics::Frustum& getFrustum() const {
        if (_frustumInvalid) {
            _frustum.calculate(_projectionMatrix, _viewMatrix);
            _frustumInvalid = false;
        }
        return _frustum;
    }
    inline const orientation_t& getOrientation() const { return _ori; }
    inline CameraTurnMode getTurnMode() const { return _turnMode; }
    inline const Vector3f& getTrackPosition() const { return _trackPos; }
    /**
     * @brief
     *  Get the center of the camera.
     * @return
     *  the center of the camera
     */
    inline const Vector3f& getCenter() const { return _center; }
    inline uint8_t getTurnTime() const { return _turnTime; }
    inline float getTurnZ_turns() const { return _turnZ_turns; }
    inline float getTurnZ_radians() const { return _turnZ_radians; }


    inline float getMotionBlur() const { return _motionBlur; }
    inline float getMotionBlurOld() const { return _motionBlurOld; }
    inline int getSwing() const { return _swing; }

    inline int getLastFrame() const {return _lastFrame;}
    inline std::shared_ptr<Ego::Graphics::TileList> getTileList() const {return _tileList;}
    inline std::shared_ptr<Ego::Graphics::EntityList> getEntityList() const {return _entityList;}

    inline const std::forward_list<ObjectRef>& getTrackList() const { return _trackList; }

    inline const ego_frect_t& getScreen() const { return _screen; }
    
    /**
     * @brief
     *  Sets which areas of the video screen this camera should draw on.
     * @param xmin, ymin, xmax, ymax
     *  the extends of the screen
     */
    void setScreen(float xmin, float ymin, float xmax, float ymax);
    
    /**
     * @brief
     *  Makes this camera track the specified target.
     * @param targetRef
     *  the target
     */
    void addTrackTarget(ObjectRef targetRef);

    /// @details This function moves the camera
    void update(const ego_mesh_t *mesh);

    /**
     * @brief
     *  Set which frame this camera was last updated.
     * @param frame
     *  the frame
     */
    void setLastFrame(int frame)
    {
        _lastFrame = frame;
    }

    /**
     * @brief
     *  Makes sure the camera starts in a suitable position.
     */
    void reset(const ego_mesh_t *mesh);

    /**
     * @brief
     *  Force the camera to focus in on the players. Should be called any time there is
     *  a "change of scene". With the new velocity-tracking of the camera, this would include
     *  things like character respawns, adding new players, etc.
     */
    void resetTarget(const ego_mesh_t *mesh);

    /**
     * @brief
     *  Set the camera movement mode.
     * @param mode
     *  the camera movement mode
     */
    void setCameraMovementMode(CameraMovementMode mode)
    {
        _moveMode = mode;
    }

protected:
    /**
     * @brief
     *  This function makes the camera turn to face the character.
	 */
	void updatePosition();

	void updateCenter();

	void updateTrack(const ego_mesh_t * mesh);

	/**
	 * @brief
     *  Update special effects like grog, blur, shaking, etc.
	 */
	void updateEffects();

    /**
     * @brief
     *  Make the camera look downwards as it is raised up.
	 */
	void updateZoom();

    /**
     * @brief
     *  This function sets pcam->mView to the camera's location and rotation.
	 */
	void makeMatrix();

    /**
     * @brief
     *  Read camera control input for one specific player controller.
	 */
	void readInput(input_device_t *pdevice);

	/**
	 * @brief
     *  Helper function to calculate FOV.
	 */
	static inline float multiplyFOV(const float old_fov_deg, const float factor);

	void resetView();

	void updateProjection(const float fov_deg, const float aspect_ratio, const float frustum_near = 1.0f, const float frustum_far = 20.0f);

private:
	const CameraOptions _options;



    // The view frustum.
    mutable Ego::Graphics::Frustum _frustum;
    /**
     * @brief
     *  If either the projection or the view matrix changed, the frustum is marked as invalid,
     *  and - if it is requested - needs to be recomputed.
     */
    mutable bool _frustumInvalid;

    // How movement is calculated.
    CameraMovementMode  _moveMode;   ///< The camera movement mode.

    // How turning is calculated.
	CameraTurnMode _turnMode;   ///< The camera turn mode.
    uint8_t        _turnTime;   ///< Time for the smooth turn.


    orientation_t  _ori;        ///< @brief The camera orientation.

    // The middle of the objects that are being tracked.
	Vector3f _trackPos;         ///< Trackee position.
    float    _trackLevel;

    // The position that the camera is focused on.
    float     _zoom;    ///< Distance from the center
	Vector3f  _center;  ///< The position that the camera is focused on.

    // Camera z movement.
    float _zadd;     ///< Camera height above terrain.
    float _zaddGoto; ///< Desired z position.
    float _zGoto;    ///< Effective z position.

    // Turning
    float _turnZ_radians;   ///< Camera z rotation (in radians).
    float _turnZ_turns;     ///< Camera z rotation (in turns).
    float _turnZAdd;        ///< Turning rate.
    float _turnZSustain;    ///< Turning rate falloff.


    // Effects
    float _motionBlur;         ///< Blurry effect.
    float _motionBlurOld;      ///< Blurry effect one frame ago (used to init the accum buffer).
    int   _swing;              ///< Camera swingin'.
    int   _swingRate;
    float _swingAmp;
    float _roll;

    // Extended camera data.
    std::forward_list<ObjectRef> _trackList;  ///< List of objects this camera is tracking.
    ego_frect_t                  _screen;

    int _lastFrame;         ///< Number of last update frame.
    std::shared_ptr<Ego::Graphics::TileList> _tileList;     ///< A pointer to a tile list or a null pointer.
    std::shared_ptr<Ego::Graphics::EntityList> _entityList; ///< A pointer to an entity list or a null pointer.
};
