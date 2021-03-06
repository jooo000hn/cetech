for k, v in pairs(cetech) do _G[k] = v end

require 'cetech/world'
require 'cetech/entity'
require 'cetech/transform'
require 'cetech/scenegraph'
require 'cetech/keyboard'
require 'cetech/mouse'
require 'cetech/gamepad'
require 'cetech/mesh'
require 'cetech/material'
require 'cetech/renderer'
require 'cetech/level'
require 'cetech/application'
require 'cetech/resource'
require 'cetech/component'

require 'core/fpscamera'

Game = Game or {}

local quit_btn = Keyboard.button_index 'q'
local debug_btn = Keyboard.button_index 'f9'
local reload_btn = Keyboard.button_index 'r'
local capture_btn = Keyboard.button_index 'f10'
local screenshot_btn = Keyboard.button_index 'f11'
local log_test_btn = Keyboard.button_index 'l'
local flymode_btn = Keyboard.button_index 'lshift'

function Game:init()
    Log.info("boot.lua", "Platform %s", Application.get_platform())

    Log.info("boot.lua", "info")
    Log.warning("boot.lua", "warn")
    Log.error("boot.lua", "error")
    Log.debug("boot.lua", "debug")

    --    self.viewport = Renderer.GetViewport("default")
    self.world = World.create()

    --    self.entity = Entity.
    -- -- (self.world, "entity1");
    --    local t = Transform.get(self.world, self.entity)
    --    local p = Transform.get_scale(t)
    --    Log.debug("lua", "%f %f %f", p.x, p.y, p.z)

    --    self.level = World.LoadLevel(self.world, "level1");
    --    self.level = World.LoadLevel(self.world, "level1",
    --                 Vec3f.make(2, 5.0, 0.0),
    --                 Quatf.Identity, Vec3f.Unit);

    --    self.level_entity = World.LevelEntity(self.world, self.level)

    --    self.entity2 = World.EntityByName(self.world, self.level, "box2")
    --    self.entity = self.entity1

    self.camera_entity = Entity.spawn(self.world, "camera");

--    a =  Transform.get(self.world, self.camera_entity)
--    Log.info("game", "%s", tostring(a))

    self.camera = 0; --Camera.GetCamera(self.world, self.camera_entity);
    self.fps_camera = FPSCamera(self.world, self.camera_entity)
    --Entity.spawn(self.world, "entity11");

    self.debug = false
    self.capture = false
    self.switch_entity = false

    self.level = Level.load_level(self.world, "level1")
    self.entity = Level.entity_by_id(self.level, "55643423443313252");

--    Component.set_property(self.world, self.entity, "transform", "position", Vec3f.make(33.0, 22.0, 10.0))
--    local v = Component.get_property(self.world, self.entity, "transform", "position")
--
--    Log.info("dsadsadsadsa", "%f %f %f", v.x, v.y, v.z)

    --Entity.destroy(self.world, self.entity)
end

function Game:shutdown()
    Log.info("boot.lua", "shutdown")

--    Entity.destroy(self.world, self.camera_entity);

    Level.destroy(self.world, self.level);
    World.destroy(self.world);
end

function rotator(world, entity, node_name, delta_rot)
    local node = SceneGraph.node_by_name(world, entity, node_name)
    local rot = SceneGraph.get_rotation(node)
    rot = rot * delta_rot
    SceneGraph.set_rotation(node, rot)
end

function transform_rotator(world, entity, delta_rot)
    local lt = Transform.get(world, entity)
    local rot = Transform.get_rotation(lt)
    rot = rot * delta_rot

    Transform.set_rotation(lt, rot)
end

TEXTURE_CYCLE = { "content/scene/m4a1/m4_diff", "content/uv_checker", "content/texture1" }
TEXTURE_CYCLE_IT = 1

L = 2
function Game:update(dt)
    -- Application.quit()

    local mesh = Mesh.get(self.world, self.entity)
    local material = Mesh.get_material(mesh)

--    L = L + dt * 0.1
--    if (L >= 1.0) then L = 0; end

    Material.set_vec4f(material, "u_vec4", Vec4f.make(L, L, L, 1.0))

    if Keyboard.button_pressed(Keyboard.button_index("t")) then
        Material.set_texture(material, "u_texColor", TEXTURE_CYCLE[(TEXTURE_CYCLE_IT % #TEXTURE_CYCLE) + 1])


        TEXTURE_CYCLE_IT = TEXTURE_CYCLE_IT + 1
    end

    local level_entity = Level.entity(self.level)
    --    transform_rotator(self.world, level_entity, Quatf.from_axis_angle(Vec3f.unit_y(), 0.02))
    rotator(self.world, Level.entity_by_id(self.level, "55643433135454252"), "n_cube", Quatf.from_axis_angle(Vec3f.unit_x(), 0.05))
    --transform_rotator(self.world, self.entity, Quatf.from_axis_angle(Vec3f.unit_z(), 0.08))

    if Keyboard.button_pressed(reload_btn) then
        ResourceManager.compile_all()
        ResourceManager.reload_all()
        Module.reload_all()
    end

    if Keyboard.button_pressed(quit_btn) then
        Application.quit()
    end

    if Keyboard.button_pressed(debug_btn) then
        self.debug = not self.debug;
    end
        Renderer.set_debug(self.debug)

    if Keyboard.button_pressed(capture_btn) then
        self.capture = not self.capture;

        if self.capture then
            RenderSystem.BeginCapture()
        else
            RenderSystem.EndCapture()
        end
    end

    if Keyboard.button_pressed(screenshot_btn) then
        RenderSystem.SaveScreenShot("screenshot");
    end

    if Keyboard.button_pressed(Keyboard.button_index('f8')) then
        self.switch_entity = not self.switch_entity
        if self.switch_entity then
            self.entity = self.level_entity
        else
            self.entity = self.entity1
        end
    end

    if Keyboard.button_state(log_test_btn) then
        Log.info("game", "INFO TEST")
        Log.warning("game", "WARN TEST")
        Log.error("game", "ERROR TEST")
        Log.debug("game", "DEBUG TEST")
    end

    if Keyboard.button_state(flymode_btn) then
        self.fps_camera.fly_mode = true
    else
        self.fps_camera.fly_mode = false
    end

    local dx = 0
    local dy = 0
    if Mouse.button_state(Mouse.button_index("left")) then
        local m_axis = Mouse.axis(Mouse.axis_index("relative"))
        dx, dy = m_axis.x, m_axis.y
        if dx ~= 0 or dy ~= 0 then
            --  Log.debug("lua", "%f %f", dx, dy)
        end

        m_axis = Mouse.axis(Mouse.axis_index("absolute"))
        local x, y = m_axis.x, m_axis.y
        --Log.debug("lua", "abs: %f %f", x, y)
    end

    local up = Keyboard.button_state(Keyboard.button_index('w'))
    local down = Keyboard.button_state(Keyboard.button_index('s'))
    local left = Keyboard.button_state(Keyboard.button_index('a'))
    local right = Keyboard.button_state(Keyboard.button_index('d'))

    local updown = 0.0
    local leftdown = 0.0
    if up then updown = 20.0 end
    if down then updown = -20.0 end
    if left then leftdown = -20.0 end
    if right then leftdown = 20.0 end

    self.fps_camera:update(dt, dx, dy, updown, leftdown)

    if Gamepad.is_active(0) then
        if Gamepad.button_pressed(0, Gamepad.button_index("left_shoulder")) then
            Gamepad.play_rumble(0, 5.0, 1000)
        end

        local right_a = Gamepad.axis(0, Gamepad.axis_index("right"))
        local left_a = Gamepad.axis(0, Gamepad.axis_index("left"))
        -- -- Log.info("lua", "{0}, {1}, {2}", left_a.X, left_a.Y, left_a.Z)

        if Gamepad.button_state(0, Gamepad.button_index("right_shoulder")) then
            self.fps_camera.fly_mode = true
        else
            self.fps_camera.fly_mode = false
        end

        self.fps_camera:update(dt, right_a.x * 1.0, right_a.y * 1.0, left_a.y * 20, left_a.x * 20)
    end

    World.update(self.world, dt)
end

function Game:render()
    Renderer.render_world(self.world, self.fps_camera.camera, self.viewport)
end

function foo(value)
    return value
end
