#pragma once

#ifndef DISABLE_PLAYFABENTITY_API

#include <playfab/PlayFabEventPipeline.h>

#include <memory>
#include <unordered_map>

namespace PlayFab
{
    /// <summary>
    /// The enumeration of all built-in event pipelines
    /// </summary>
    enum class EventPipelineKey
    {
        PlayFabPlayStream, // PlayFab (PlayStream) event pipeline
        PlayFabTelemetry // PlayFab event pipeline, bypasses PlayStream
    };

    // Workaround for c++11
    // https://stackoverflow.com/questions/18837857/cant-use-enum-class-as-unordered-map-key
    struct _EventPipelineKeyHash
    {
        template <typename T>
        std::size_t operator()(T t) const { return static_cast<std::size_t>(t); }
    };

    /// <summary>
    /// Interface for any event router
    /// </summary>
    class IPlayFabEventRouter
    {
    public:
        virtual ~IPlayFabEventRouter() {}
        virtual void RouteEvent(std::shared_ptr<const IPlayFabEmitEventRequest> request) const = 0; // Route an event to pipelines. This method must be thread-safe.
        const std::unordered_map<EventPipelineKey, std::shared_ptr<IPlayFabEventPipeline>, _EventPipelineKeyHash>& GetPipelines() const;

        virtual void Update() = 0;
    protected:
        std::unordered_map<EventPipelineKey, std::shared_ptr<IPlayFabEventPipeline>, _EventPipelineKeyHash> pipelines;
    };

    /// <summary>
    /// Default implementation of event router
    /// </summary>
    class PlayFabEventRouter : public IPlayFabEventRouter
    {
    public:
        PlayFabEventRouter(bool threadedEventPipeline);
        virtual void RouteEvent(std::shared_ptr<const IPlayFabEmitEventRequest> request) const override;

        /// <summary>
        /// Updates underlying PlayFabEventPipeline
        /// This function must be called every game tick if threadedEventPipeline is set to false
        /// </summary>
        virtual void Update() override;
    private:
    };
}

#endif
