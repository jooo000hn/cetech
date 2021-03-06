//! \addtogroup World
//! \{
#ifndef CETECH_LEVEL_H
#define CETECH_LEVEL_H

#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
// Includes
//==============================================================================

struct ct_world;
struct ct_entity;


//==============================================================================
// Typedefs
//==============================================================================

//! Level idx
struct ct_level {
    uint32_t idx;
};


//==============================================================================
// Api
//==============================================================================

//! Level API V0
struct ct_level_a0 {

    //! Load level from resource
    //! \param world World
    //! \param name Resource name
    //! \return New level
    struct ct_level (*load_level)(struct ct_world world,
                                  uint64_t name);

    //! Destroy level
    //! \param world World
    //! \param level Level
    void (*destroy)(struct ct_world world,
                    struct ct_level level);

    //! Get entity in level by name
    //! \param level Level
    //! \param name Name
    //! \return Entity
    struct ct_entity (*entity_by_id)(struct ct_level level,
                                     uint64_t name);

    //! Get level entity
    //! \param level Level
    //! \return Level entity
    struct ct_entity (*entity)(struct ct_level level);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_LEVEL_H
//! |}