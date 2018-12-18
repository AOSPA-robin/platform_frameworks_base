/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FRAMEWORKS_BASE_WEBVIEWFUNCTOR_H
#define FRAMEWORKS_BASE_WEBVIEWFUNCTOR_H

#include <private/hwui/DrawGlInfo.h>

namespace android::uirenderer {

enum class RenderMode {
    OpenGL_ES,
    Vulkan,
};

// Static for the lifetime of the process
RenderMode WebViewFunctor_queryPlatformRenderMode();

struct WebViewSyncData {
    bool applyForceDark;
};

struct WebViewFunctorCallbacks {
    // kModeSync, called on RenderThread
    void (*onSync)(int functor, const WebViewSyncData& syncData);

    // Called when either the context is destroyed _or_ when the functor's last reference goes
    // away. Will always be called with an active context and always on renderthread.
    void (*onContextDestroyed)(int functor);

    // Called when the last reference to the handle goes away and the handle is considered
    // irrevocably destroyed. Will always be proceeded by a call to onContextDestroyed if
    // this functor had ever been drawn.
    void (*onDestroyed)(int functor);

    union {
        struct {
            // Called on RenderThread. initialize is guaranteed to happen before this call
            void (*draw)(int functor, const DrawGlInfo& params);
        } gles;
        // TODO: VK support. The current DrawVkInfo is monolithic and needs to be split up for
        // what params are valid on what callbacks
        struct {
            // Called either the first time the functor is used or the first time it's used after
            // a call to onContextDestroyed.
            // void (*initialize)(int functor, const InitParams& params);
            // void (*frameStart)(int functor, /* todo: what params are actually needed for this to
            // be useful? Is this useful? */)
            // void (*draw)(int functor, const CompositeParams& params /* todo: rename - composite
            // almost always means something else, and we aren't compositing */);
            // void (*frameEnd)(int functor, const PostCompositeParams& params /* todo: same as
            // CompositeParams - rename */);
        } vk;
    };
};

// Creates a new WebViewFunctor from the given prototype. The prototype is copied after
// this function returns. Caller retains full ownership of it.
// Returns -1 if the creation fails (such as an unsupported functorMode + platform mode combination)
int WebViewFunctor_create(const WebViewFunctorCallbacks& prototype, RenderMode functorMode);

// May be called on any thread to signal that the functor should be destroyed.
// The functor will receive an onDestroyed when the last usage of it is released,
// and it should be considered alive & active until that point.
void WebViewFunctor_release(int functor);

}  // namespace android::uirenderer

#endif  // FRAMEWORKS_BASE_WEBVIEWFUNCTOR_H
