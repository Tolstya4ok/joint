#ifndef SRC_BENCHMARKS_BENCHMARKS_EXCEPTION_HPP
#define SRC_BENCHMARKS_BENCHMARKS_EXCEPTION_HPP


#include <benchmarks/BenchmarksBase.hpp>


namespace benchmarks
{

	template < typename Desc_ >
	class ExceptionBenchmarks : public BenchmarksBase<Desc_, typename Desc_::ExceptionBenchmarksPtr>
	{
		using Base = BenchmarksBase<Desc_, typename Desc_::ExceptionBenchmarksPtr>;
		using Base::AddSimpleBenchmark;
		using BenchmarkCtx = typename Desc_::BenchmarkCtx;

	public:
		ExceptionBenchmarks()
			: Base("exception", [](const BenchmarkCtx& ctx) { return ctx.CreateExceptionBenchmarks(); })
		{
			using i64 = int64_t;
			using C = typename Desc_::BenchmarkCtx;
			using B = typename Desc_::ExceptionBenchmarksPtr;

			AddSimpleBenchmark("throwNative", [](C& ctx, i64 n, B b){ b->MeasureNativeThrow(n); });
			AddSimpleBenchmark("throwComponentSide", [](C& ctx, i64 n, B b){
				for (int64_t i = 0; i < n; ++i)
				{
					try { b->Throw(); } // TODO: Use exception-free C++ joint wrappers
					catch (const std::exception&) { }
				}
			});
			AddSimpleBenchmark("throwProxySide", [](C& ctx, i64 n, B b){ b->MeasureProxySideThrow(ctx.CreateLocalThrower(), n); });
		}
	};

}

#endif