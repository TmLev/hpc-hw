#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

////////////////////////////////////////////////////////////////////////////////

using Int = std::int64_t;
using U8 = std::uint8_t;
using Index = std::size_t;
using Size = std::size_t;
using Float = double;

template <typename T>
using Vector = std::vector<T>;

template <typename T>
using Matrix = Vector<Vector<T>>;

using CpuClock = std::chrono::steady_clock;
using CpuMs = std::chrono::milliseconds;
using CpuDurations = Vector<std::result_of_t<decltype (&CpuMs::count)(CpuMs)>>;

using GpuMs = float;
using GpuDurations = Vector<GpuMs>;

////////////////////////////////////////////////////////////////////////////////

#if !defined(_LIBCPP_STD_VER) || _LIBCPP_STD_VER <= 14

namespace std {

template <class _Tp, class _Compare>
const _Tp& clamp(const _Tp& __v, const _Tp& __lo, const _Tp& __hi,
                 _Compare __comp) {
  return __comp(__v, __lo) ? __lo : __comp(__hi, __v) ? __hi : __v;
}

template <class _Tp>
const _Tp& clamp(const _Tp& __v, const _Tp& __lo, const _Tp& __hi) {
  return clamp(__v, __lo, __hi, std::less<_Tp>());
}

}  // namespace std

#endif  // !defined(_LIBCPP_STD_VER) || _LIBCPP_STD_VER <= 14

////////////////////////////////////////////////////////////////////////////////

#define COORD_TO_1D(x, y, width) ((y)*width + (x))
#define ROW_COL_TO_1D(row, col, width) ((row)*width + (col))

////////////////////////////////////////////////////////////////////////////////

namespace gauss {

auto Gaussian(Float x, Float mu, Float sigma) -> Float {
  const auto fraction = (x - mu) / sigma;
  return std::exp(-std::pow(fraction, 2) / 2);
}

class Kernel {
 public:
  struct Parameters {
    Size size;
    Float sigma;
  };

 public:
  explicit Kernel(const Parameters& params)
      : size_(params.size), sigma_(params.sigma), data_(size_ * size_, 0) {
    auto sum = Float{0};
    const auto mu = static_cast<Float>(static_cast<Int>(size_) / 2);

    for (auto row = Index{0}; row < size_; ++row) {
      for (auto col = Index{0}; col < size_; ++col) {
        (*this)(row, col) = Gaussian(static_cast<Float>(row), mu, sigma_) *
                            Gaussian(static_cast<Float>(col), mu, sigma_) /
                            (2 * M_PI * std::pow(sigma_, 2));
        sum += (*this)(row, col);
      }
    }

    for (auto row = Index{0}; row < size_; ++row) {
      for (auto col = Index{0}; col < size_; ++col) {
        (*this)(row, col) /= sum;
      }
    }
  }

  auto operator()(Index row, Index col) -> Float& {
    return data_[ROW_COL_TO_1D(row, col, size_)];
  }

  auto operator()(Index row, Index col) const -> const Float& {
    return data_[ROW_COL_TO_1D(row, col, size_)];
  }

  auto ToString(const std::string& delim = " ") const -> std::string {
    auto ss = std::stringstream{};

    for (auto x = Index{0}; x < size_; ++x) {
      for (auto y = Index{0}; y < size_; ++y) {
        ss << (*this)(x, y) << delim;
      }
      ss << '\n';
    }

    return ss.str();
  }

  auto Size() const -> Index {
    return size_;
  }

  auto Data() -> Float* {
    return data_.data();
  }

  auto Data() const -> const Float* {
    return data_.data();
  }

 private:
  Index size_;
  Float sigma_;
  Vector<Float> data_;
};

}  // namespace gauss

////////////////////////////////////////////////////////////////////////////////

class Image {
 public:
  struct Parameters {
    Size padding;
    std::string path_to_text;
  };

 public:
  explicit Image(const Parameters& params) : padding_(params.padding) {
    auto file = std::fstream{params.path_to_text};

    // Read properties
    file >> width_ >> height_ >> channels_;

    // Resize matrices for every channel
    data_.resize(channels_);
    for (auto& channel : data_) {
      channel.resize(PaddedHeight() * PaddedWidth());
    }

    // Read RGB data
    for (auto channel = Index{0}; channel < channels_; ++channel) {
      for (auto row = Index{padding_}; row < height_ + padding_; ++row) {
        for (auto col = Index{padding_}; col < width_ + padding_; ++col) {
          auto pixel = Int{};
          file >> pixel;
          (*this)(channel, row, col) = static_cast<U8>(pixel);
        }
      }
    }

    // Pad with adjacent pixels
    for (auto channel = Index{0}; channel < channels_; ++channel) {
      // Highest and lowest rows
      for (auto row = Index{0}; row < padding_; ++row) {
        for (auto col = Index{0}; col < PaddedWidth(); ++col) {
          const auto r = std::clamp(row, 0 + padding_, height_ - 1 + padding_);
          const auto c = std::clamp(col, 0 + padding_, width_ - 1 + padding_);

          const auto high = row;
          const auto low = row + height_ + padding_;

          (*this)(channel, high, col) = (*this)(channel, r, c);
          (*this)(channel, low, col) =
              (*this)(channel, PaddedHeight() - 1 - r, c);
        }
      }

      // Leftmost and rightmost columns
      for (auto row = Index{0}; row < PaddedHeight(); ++row) {
        for (auto col = Index{0}; col < padding_; ++col) {
          const auto r = std::clamp(row, 0 + padding_, height_ - 1 + padding_);
          const auto c = std::clamp(col, 0 + padding_, width_ - 1 + padding_);

          const auto left = col;
          const auto right = col + width_ + padding_;

          (*this)(channel, row, left) = (*this)(channel, r, c);
          (*this)(channel, row, right) =
              (*this)(channel, r, PaddedWidth() - 1 - c);
        }
      }
    }
  }

  // Iterate

  auto operator()(Index channel, Index row, Index col) -> U8& {
    const auto width = PaddedWidth();
    return data_[channel][ROW_COL_TO_1D(row, col, width)];
  }

  auto operator()(Index channel, Index row, Index col) const -> const U8& {
    const auto width = PaddedWidth();
    return data_[channel][ROW_COL_TO_1D(row, col, width)];
  }

  // Access

  auto PaddedHeight() const -> Size {
    return height_ + 2 * padding_;
  }

  auto PaddedWidth() const -> Size {
    return width_ + 2 * padding_;
  }

  auto Padding() const -> Size {
    return padding_;
  }

  auto Channels() const -> Size {
    return channels_;
  }

  auto ChannelData(Index channel) -> U8* {
    return data_[channel].data();
  }

  // Save

  auto SaveTo(const std::string& path_to_text, bool padded = true) const
      -> void {
    const auto mode = std::fstream::out | std::fstream::trunc;
    auto file = std::fstream{path_to_text, mode};

    const auto width = padded ? PaddedWidth() : width_;
    const auto height = padded ? PaddedHeight() : height_;

    file << width << ' ' << height << ' ' << channels_ << '\n';

    const auto row_from = padded ? Index{0} : padding_;
    const auto row_to = padded ? PaddedHeight() : height_ + padding_;

    const auto col_from = padded ? Index{0} : padding_;
    const auto col_to = padded ? PaddedWidth() : width_ + padding_;

    for (auto channel = Index{0}; channel < channels_; ++channel) {
      for (auto row = row_from; row < row_to; ++row) {
        for (auto col = col_from; col < col_to; ++col) {
          file << std::to_string((*this)(channel, row, col)) << ' ';
        }
        file << '\n';
      }
    }

    file.flush();
  }

  // Kernel multiplication

  auto operator*=(const gauss::Kernel& kernel) -> void {
    for (auto channel = Index{0}; channel < channels_; ++channel) {
      auto chan = Vector<U8>(PaddedHeight() * PaddedWidth(), 0);

      for (auto row = padding_; row < height_ + padding_; ++row) {
        for (auto col = padding_; col < width_ + padding_; ++col) {
          const auto half = static_cast<Int>(kernel.Size()) / 2;
          auto blur = Float{0};

          for (auto dy = -half; dy <= half; ++dy) {
            for (auto dx = -half; dx <= half; ++dx) {
              const auto y = static_cast<Int>(row) + dy;
              const auto x = static_cast<Int>(col) + dx;

              blur += kernel(half + dy, half + dx) * (*this)(channel, y, x);
            }
          }

          chan[ROW_COL_TO_1D(row, col, PaddedWidth())] = static_cast<U8>(blur);
        }
      }

      data_[channel] = chan;
    }
  }

 private:
  Size width_{0};
  Size height_{0};
  Size channels_{3};
  Size padding_{1};
  Matrix<U8> data_;
};

////////////////////////////////////////////////////////////////////////////////

auto CpuBlur(const std::string& path, const gauss::Kernel& kernel) {
  // Prepare image
  auto image = Image({kernel.Size() / 2, path});

  // Start timer
  const auto start = CpuClock::now();

  // Blur image
  image *= kernel;

  // Stop timer
  const auto duration =
      std::chrono::duration_cast<CpuMs>(CpuClock::now() - start).count();

  // Save image as text
  image.SaveTo(std::string{"cpu-blurred-"} + path, /*padded=*/false);

  return duration;
}

////////////////////////////////////////////////////////////////////////////////

#ifdef __NVCC__

inline void CudaAssert(cudaError_t code, int line, bool abort = true) {
  if (code != cudaSuccess) {
    std::cerr << "CudaAssert: " << cudaGetErrorString(code) << ", line " << line
              << '\n';
    if (abort) {
      std::abort();
    }
  }
}

#define EXPECT_OK(ans) \
  { CudaAssert((ans), __LINE__); }

__global__ void ApplyBlur(const uint8_t* input, uint8_t* output,
                          uint64_t height, uint64_t width, uint64_t padding,
                          const double* kernel, uint64_t kernel_size) {
  const int row = blockIdx.y * blockDim.y + threadIdx.y;
  const int col = blockIdx.x * blockDim.x + threadIdx.x;

  if (row < padding || row > height - 1 - padding || col < padding ||
      col > width - 1 - padding) {
    return;
  }

  double blur = 0;

  const int half = kernel_size / 2;
  for (int dy = -half; dy <= half; ++dy) {
    for (int dx = -half; dx <= half; ++dx) {
      int y = row + dy;
      int x = col + dx;

      int kernel_index = COORD_TO_1D(half + dx, half + dy, kernel_size);
      int input_index = COORD_TO_1D(x, y, width);

      blur += kernel[kernel_index] * input[input_index];
    }
  }

  output[ROW_COL_TO_1D(row, col, width)] = blur;
}

auto GpuBlur(const std::string& path, const gauss::Kernel& kernel) -> GpuMs {
  // Measurements setup
  auto start = cudaEvent_t{};
  auto end = cudaEvent_t{};
  EXPECT_OK(cudaEventCreate(&start));
  EXPECT_OK(cudaEventCreate(&end));

  // Prepare image
  auto image = Image({kernel.Size() / 2, path});

  // Allocate memory for channels
  auto size = sizeof(uint8_t) * image.PaddedHeight() * image.PaddedWidth();

  auto channels = Vector<uint8_t*>(image.Channels(), nullptr);
  for (auto& channel : channels) {
    EXPECT_OK(cudaMalloc(&channel, size));
  }

  auto blurred_channels = Vector<uint8_t*>(image.Channels(), nullptr);
  for (auto& channel : blurred_channels) {
    EXPECT_OK(cudaMalloc(&channel, size));
  }

  // Allocate memory for kernel
  auto kernel_size = sizeof(double) * kernel.Size() * kernel.Size();
  double* device_kernel = nullptr;
  EXPECT_OK(cudaMalloc(&device_kernel, kernel_size));

  // Copy data
  for (auto c = Index{0}; c < image.Channels(); ++c) {
    EXPECT_OK(cudaMemcpy(/*dst=*/channels[c], /*src=*/image.ChannelData(c),
                         size, cudaMemcpyHostToDevice));
  }

  EXPECT_OK(cudaMemcpy(/*dst=*/device_kernel, /*src=*/kernel.Data(),
                       kernel_size, cudaMemcpyHostToDevice));

  // Start timer
  EXPECT_OK(cudaEventRecord(start));

  // Blur image
  const auto threads_per_block = dim3{16, 16};
  const auto num_blocks = dim3{
      image.PaddedWidth() / threads_per_block.x + 1,
      image.PaddedHeight() / threads_per_block.y + 1,
  };
  for (auto c = Index{0}; c < image.Channels(); ++c) {
    ApplyBlur<<<num_blocks, threads_per_block>>>(
        /*input=*/channels[c],
        /*output=*/blurred_channels[c],

        /*height=*/image.PaddedHeight(),
        /*width=*/image.PaddedWidth(),
        /*padding=*/image.Padding(),

        /*kernel=*/device_kernel,
        /*kernel_size=*/kernel.Size());
  }

  // Stop timer
  EXPECT_OK(cudaEventRecord(end));

  // Copy blurred channels to image
  for (auto c = Index{0}; c < image.Channels(); ++c) {
    EXPECT_OK(cudaMemcpy(/*dst=*/image.ChannelData(c),
                         /*src=*/blurred_channels[c], size,
                         cudaMemcpyDeviceToHost));
  }

  // Free memory
  for (auto channel : channels) {
    EXPECT_OK(cudaFree(channel));
  }

  for (auto channel : blurred_channels) {
    EXPECT_OK(cudaFree(channel));
  }

  EXPECT_OK(cudaFree(device_kernel));

  // Record measurement
  EXPECT_OK(cudaEventSynchronize(end));
  auto duration = GpuMs{};
  EXPECT_OK(cudaEventElapsedTime(&duration, start, end));

  // Save image as text
  image.SaveTo(std::string{"gpu-blurred-"} + path, /*padded=*/false);

  return duration;
}

#endif  // __NVCC__

////////////////////////////////////////////////////////////////////////////////

constexpr auto kDurDelim = ", ";

template <typename Dur>
auto operator<<(std::ostream& out, const Vector<Dur>& durations)
    -> std::ostream& {
  for (const auto& dur : durations) {
    out << dur << kDurDelim;
  }
  return out;
}

////////////////////////////////////////////////////////////////////////////////

constexpr auto kKernelSize = 5;

// clang-format off

const auto kPaths = Vector<std::string>{
    "01-test.txt",
    "02-eyes.txt",
    "03-rsm.txt",
    "04-mpi-code.txt",
    "05-keyboard.txt",
    "06-tenet.txt",
    "07-mountain.txt",
};

// clang-format on

////////////////////////////////////////////////////////////////////////////////

auto main() -> int {
  const auto kernel = gauss::Kernel({kKernelSize, 1});
  std::cout << "Gaussian kernel: \n" << kernel.ToString() << '\n';

  // CPU blurs

  auto cpu_durations = CpuDurations{};
  for (const auto& path : kPaths) {
    cpu_durations.push_back(CpuBlur(path, kernel));
  }

  std::cout << "CPU durations (milliseconds):" << '\n' << cpu_durations << '\n';

#ifdef __NVCC__
  // GPU blurs

  auto gpu_durations = GpuDurations{};
  for (const auto& path : kPaths) {
    gpu_durations.push_back(GpuBlur(path, kernel));
  }

  std::cout << "GPU durations (milliseconds):" << '\n' << gpu_durations << '\n';
#endif  // __NVCC__
}
