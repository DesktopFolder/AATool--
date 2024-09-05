#pragma once

namespace aa
{
struct AllAdvancements;
struct AllAdvancements;

// Observer pointer... where are you... :sob:
template<typename T>
using weak_ptr = T*;

struct Event {};

// What kind of events can we have?
struct ManifestUpdate : Event
{
    const weak_ptr<AllAdvancements> new_manifest;
};

struct StatusUpdate : Event
{
    const weak_ptr<AllAdvancements> new_status;
};

struct EventHandler
{
    virtual void handle_event(ManifestUpdate) {}
    virtual void handle_event(StatusUpdate) {}
};
}