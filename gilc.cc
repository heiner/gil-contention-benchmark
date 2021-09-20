
#include <pybind11/pybind11.h>

namespace py = pybind11;

class Gilc {
public:
  Gilc() : calls_(0) {}
  double gilc() {
    ++calls_;
    if (calls_ % 1000 == 1) {
      auto now = std::chrono::system_clock::now();
      std::chrono::duration<double> delta = now - last_time_;
      last_time_ = std::move(now);
      if (calls_ > 1) // Not first time.
        return 1000.0 / delta.count();
    }
    return 0.0;
  }

private:
  int64_t calls_;
  std::chrono::time_point<std::chrono::system_clock> last_time_;
};

PYBIND11_MODULE(gilc, m) {
  py::class_<Gilc>(m, "Gilc")
      .def(py::init<>())
      .def("gilc", &Gilc::gilc)
      .def("gilc_nogil", &Gilc::gilc, py::call_guard<py::gil_scoped_release>());
}
