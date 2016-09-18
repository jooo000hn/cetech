﻿using CETech.CEMath;

namespace CETech.World
{
    public partial class World
    {
        public static readonly int NullWorld = 0;

        public static void Init()
        {
            InitImpl();
        }

        public static void Shutdown()
        {
            ShutdownImpl();
        }

        public static int CreateWorld()
        {
            return CreateWorldImpl();
        }

        public static void DestroyWorld(int world)
        {
            DestroyWorldImpl(world);
        }

        public static void Update(int world, float dt)
        {
            UpdateImpl(world, dt);
        }

        public static int LoadLevel(int world, long level, Vec3f position, Quatf rotation, Vec3f scale)
        {
            return LoadLevelImpl(world, level, position, rotation, scale);
        }

        public static int UnitByName(int world, int level, long name)
        {
            return UnitByNameImpl(world, level, name);
        }

        public static int LevelUnit(int world, int level)
        {
            return LevelUnitImpl(world, level);
        }
    }
}