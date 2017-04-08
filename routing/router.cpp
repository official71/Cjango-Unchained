#include "router.hpp"
#include "../app/externs.hpp"

inline bool registered(std::vector<std::string> patterns_list, std::string url_pattern) {
  bool found = find(patterns_list.begin(),
                    patterns_list.end(), url_pattern) != patterns_list.end();
  return found;
}

void Router::add_route(std::string url_pattern, functor f) {
  // Note: url_pattern may be a regex pattern or an actual url_path
#ifdef DYNLOAD_CJANGO
#include <algorithm>

  if (! registered(patterns_list, url_pattern))
    patterns_list.push_back(url_pattern);
  pattern_to_callback[url_pattern] = f;
  _DEBUG("updated route: ", url_pattern);

#else
  try {
    if (registered(patterns_list, url_pattern))
      throw 1;
  }
  catch (int e) {
    _DEBUG("catched: url pattern is already registered");
    return;
  }
  patterns_list.push_back(url_pattern);
  pattern_to_callback[url_pattern] = f;
#endif
  _DEBUG("add a new rule: ", url_pattern.c_str());
}

std::string Router::resolve(HttpRequest request) {
  // return its corresponding url_pattern
  // MAYBE-LATER map request.path -> url_pattern
  std::string url_pattern = request.path;
  if (registered(patterns_list, url_pattern)) {
    _DEBUG("resolve(): registered");
    return url_pattern;
  } else {
    _DEBUG("resolve(): non registered");
    return cjango::INVALIDURL;
  }
  // "resolver_match: a resolved url. This attribute is only set after URL resolving took place"
}

#ifdef DYNLOAD_CJANGO
void *Router::load_shared_object_file(const std::string& path) {
  const auto lib = dlopen(path.c_str(), RTLD_LAZY);
  if (!lib) {
    _DEBUG("Cannot load library: ", dlerror());
    std::cout <<  "Cannot load library: " << dlerror() << std::endl;
    throw "no such a object file";
    // exit(EXIT_FAILURE);
  }
  bool found = ref_count.find(path) != ref_count.end();
  if (found)
    ++ref_count[path];
  else
    ref_count[path] = 1;
  return lib;
}

void *Router::load_callback(const std::string& path, const std::string& func_name) {
  auto lib = (dlib_handler) load_shared_object_file(path);
  _DEBUG("dlopen() finished for ", path);
  bool found = std::find(dlib_handlers.begin(), dlib_handlers.end(), lib) != dlib_handlers.end();
  if (found) {
    // dlopen() returns the same (opaque) handler when specified twice
    // For updating the same-name function, we first close (as the same times as dlopen()),
    // unload the library, and then reload the updated function
    dlib_handlers.erase(std::remove(dlib_handlers.begin(), dlib_handlers.end(), lib),
                        dlib_handlers.end());
    std::cout  << "removed \n\n\n" << std::endl;
    dlclose(lib); dlclose(lib);
    ref_count[path] = ref_count[path] - 2;
    // It looks strange but this is the right way to detect same file handlers are returned
    lib = (dlib_handler) load_shared_object_file(path);
  }
  dlib_handlers.push_back(lib);
  _DEBUG("dlib_handlers.size: ", dlib_handlers.size(), "  ref_count: ", ref_count[path]);

  const auto func = dlsym(lib, func_name.c_str());
  const auto dlsym_error = dlerror();
  if (dlsym_error) {
    _DEBUG("Cannot load symbol callback()");
    std::cout <<  "Cannot load symbol: " << dlerror() << std::endl;
    throw "no such a callback name";
    // exit(EXIT_FAILURE);
  }
   // FIXME where should we invoke dlclise()?
  return func;
}

#include "../lib/json.hpp"
#include <fstream>
// third-party library for json parsing. Usage: just loading this file
// usage: github nlohmann/json
void Router::load_url_pattern_from_file() {
  std::ifstream i("callbacks/url-pattern.json");
  nlohmann::json j;
  i >> j;
  _DEBUG("loaded url-pattern.json");
  for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it) {
    const auto cinfo = it.value(); // callback info

    HttpResponse (*myfun)(HttpRequest); // function pointer

    try {
      myfun =
        (HttpResponse (*)(HttpRequest)) load_callback(cinfo["file"], cinfo["funcname"]);
    } catch (const char *e) {
      _DEBUG("aaaa\n\n\n\n\n\n\n\n");
      continue;
    }
    functor callback = myfun;
    _DEBUG(it.key(), " : ", it.value(), "\n",
           "cinfo['file']", cinfo["file"], "cinfo['funcname']", cinfo["funcname"]);
    add_route(it.key(), callback);
    // cannot dlclose here because callbacks will be used later
  }
}
#endif

HttpResponse Router::get_http_response(HttpRequest request) {
  std::string url_path = resolve(request);
  if (url_path == cjango::INVALIDURL) {
    _DEBUG("Router::get_http_response(): this HttpRequest.path is invalid");
    return HttpResponse();
  }

  functor callback = pattern_to_callback[url_path];
  _DEBUG("Router::get_http_response(): return callback");
  return callback(request);
}