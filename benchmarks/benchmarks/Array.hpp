#ifndef SRC_BENCHMARKS_BENCHMARKS_ARRAY_HPP
#define SRC_BENCHMARKS_BENCHMARKS_ARRAY_HPP


#include <CallAdapter.hpp>
#include <benchmarks/BenchmarksBase.hpp>


namespace benchmarks
{

    template < typename Desc_ >
    class ArrayBenchmarks : public BenchmarksBase<Desc_, typename Desc_::ArrayBenchmarksPtr>
    {
        using Base = BenchmarksBase<Desc_, typename Desc_::ArrayBenchmarksPtr>;
        using Base::AddSimpleBenchmark;
        using BenchmarkCtx = typename Desc_::BenchmarkCtx;

    public:
        ArrayBenchmarks()
            : Base("array", [](const BenchmarkCtx& ctx) { return ctx.CreateArrayBenchmarks(); })
        {
            using i64 = int64_t;
            using C = typename Desc_::BenchmarkCtx;
            using B = typename Desc_::ArrayBenchmarksPtr;

            // I64 benchmarks
            AddSimpleBenchmark("native_array_get_i64", [](C& ctx, i64 n, B b){ CALL( b->MeasureNativeGetI64Element(n) ); });
            AddSimpleBenchmark("native_array_set_i64", [](C& ctx, i64 n, B b){ CALL( b->MeasureNativeSetI64Element(n) ); });

            AddSimpleBenchmark("proxy_array_get_i64", [](C& ctx, i64 n, B b){ CALL( b->MeasureGetI64Element(n) ); });
            AddSimpleBenchmark("proxy_array_set_i64", [](C& ctx, i64 n, B b){ CALL( b->MeasureSetI64Element(n) ); });

            // Struct benchmarks
            AddSimpleBenchmark("native_array_get_struct", [](C& ctx, i64 n, B b){ CALL( b->MeasureNativeGetStructElement(n) ); });
            AddSimpleBenchmark("native_array_set_struct", [](C& ctx, i64 n, B b){ CALL( b->MeasureNativeSetStructElement(n) ); });

            AddSimpleBenchmark("proxy_array_get_struct", [](C& ctx, i64 n, B b){ CALL( b->MeasureGetStructElement(n) ); });
            AddSimpleBenchmark("proxy_array_set_struct", [](C& ctx, i64 n, B b){ CALL( b->MeasureSetStructElement(n) ); });

            // String benchmarks
            AddSimpleBenchmark("native_array_get_string", [](C& ctx, i64 n, B b){ CALL( b->MeasureNativeGetStringElement(n) ); });
            AddSimpleBenchmark("native_array_set_string", [](C& ctx, i64 n, B b){ CALL( b->MeasureNativeSetStringElement(n) ); });

            AddSimpleBenchmark("proxy_array_get_string", [](C& ctx, i64 n, B b){ CALL( b->MeasureGetStringElement(n) ); });
            AddSimpleBenchmark("proxy_array_set_string", [](C& ctx, i64 n, B b){ CALL( b->MeasureSetStringElement(n) ); });

            // Array benchmarks
            AddSimpleBenchmark("native_array_get_array", [](C& ctx, i64 n, B b){ CALL( b->MeasureNativeGetArrayElement(n) ); });
            AddSimpleBenchmark("native_array_set_array", [](C& ctx, i64 n, B b){ CALL( b->MeasureNativeSetArrayElement(n) ); });

            AddSimpleBenchmark("proxy_array_get_array", [](C& ctx, i64 n, B b){ CALL( b->MeasureGetArrayElement(n) ); });
            AddSimpleBenchmark("proxy_array_set_array", [](C& ctx, i64 n, B b){ CALL( b->MeasureSetArrayElement(n) ); });
        }
    };

}

#endif
