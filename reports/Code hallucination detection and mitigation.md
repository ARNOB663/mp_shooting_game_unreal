# Make perturbation earn its keep for code

You can publish this from a single 8 GB GPU, but not as "Saha et al. for code." Every mechanism in the proposed pipeline already has a published owner. Paraphrase-and-execute validation of generated code reached ICSE 2026 NIER, catching **70.6% of GPT-4o's wrong HumanEval programs at a 6.1% false-positive rate** ([ACM DL](https://dl.acm.org/doi/10.1145/3786582.3786809)). Execution clustering with abstention cuts the error rate of returned LiveCodeBench answers **from about 65% to 2%** ([arXiv 2506.11021](https://arxiv.org/abs/2506.11021)). API/package verifiers and constrained decoding are established. What nobody has published is the controlled comparison a reviewer will ask for first. At an equal sample budget, does agreement across character-, word- and sentence-level perturbed prompts predict and fix code hallucinations better than plain same-prompt resampling? And does that answer change as small local models are quantized from 8 to 2 bits? Quantization is the least crowded angle. The only code-specific hallucination study found shows **4-bit models hallucinate more packages**, even though 4-bit keeps about 99% of HumanEval accuracy ([arXiv 2512.08213](https://arxiv.org/abs/2512.08213); [arXiv 2411.02355](https://arxiv.org/pdf/2411.02355)). The strongest paper is therefore an empirical study with four parts: a decomposed hallucination profile, a matched-budget detection benchmark, Perturb–Cluster–Verify presented as a risk–coverage method with ablations, and a quantization sweep. Moving from text to code also fixes Saha et al.'s central weakness, because execution gives ground-truth correctness where HHEM gave only consistency. The realistic first targets are LLM4Code 2027 at ICSE (tentatively 13 Nov 2026) for a pilot, then ARR on 4 Jan 2027 or an MSR Registered Report leading to EMSE for the full study. One caveat applies throughout: the underlying notes came from search snippets, because arXiv, ACL and ACM were unreachable. Numbers marked [verify] must be checked against the PDFs before you cite them.

## Saha et al. measured consistency, not correctness, and code can measure both

Saha, Gupta, Chatterjee and Banerjee (Walmart Global Tech; Discover Data 3:20, May 2025) apply PromptAttack-style perturbations to queries: three strategies at each of the character, word and sentence levels. They then score the original and perturbed responses for consistency with Vectara's hallucination evaluation model (HHEM) ([Springer](https://link.springer.com/article/10.1007/s44248-025-00041-7)). Across eight models (GPT-3.5-Turbo, GPT-4, GPT-4-Turbo, Gemini-1.0-Pro, Gemini-1.5-Pro, Gemini-1.5-Flash, Llama3-8B and Gemma2-9B) and six English LongBench datasets, **GPT-4 scored highest (0.76/0.74/0.65 for character/word/sentence) and Llama3-8B lowest (0.18/0.18/0.14)** ([Springer](https://link.springer.com/article/10.1007/s44248-025-00041-7)). The design is a useful template: graded perturbation levels, no reference answers needed, one score per model. A careful reading also finds weaknesses that a code extension should treat as its to-do list:

- **No correctness check.** The paper never tests whether consistency tracks correctness.
- **No noise floor.** There is no repeated-sampling baseline, so it is unknown how much disagreement plain decoding noise would cause.
- **Unexplained aggregation.** The combined scores in Fig. 3 are lower than all three per-level scores in Fig. 1 (GPT-4: 0.595 against 0.76/0.74/0.65), which no simple average can produce.
- **Missing reporting.** Sample sizes, decoding settings and statistical tests are absent.
- **Off-label scorer.** HHEM, a document-versus-summary factual-consistency model, is used to compare one response with another.
- **An internal slip.** Section 4.2 says "six LLMs" although eight were tested.

The paper's claim to be the first perturbation-based hallucination study is also contestable. Wang and Zhu's metamorphic prompt testing for code appeared on arXiv in June 2024 ([arXiv 2406.06864](https://arxiv.org/abs/2406.06864)). No citing paper or author follow-up was found, but Google Scholar was unreachable, so that is [verify] ([Springer](https://link.springer.com/article/10.1007/s44248-025-00041-7)).

**Code removes the central problem.** A program either passes held-out tests or it does not, so the extension can report what Saha et al. could not: the correlation between perturbation agreement and actual correctness, with confidence intervals. Code also gives a principled reason to drop HHEM. A 2026 study found that NLI-style clustering puts different code samples into a single cluster. It also found most uncertainty methods exceed 0.70 AUROC on Python LiveCodeBench but mostly fall to 0.55–0.70 on Java ([arXiv 2605.28500](https://arxiv.org/html/2605.28500v1)). Another 2026 paper reports that embedding- and NLI-based semantic entropy performs only moderately on code, because text similarity misses the effect of small symbolic changes ([arXiv 2603.29292](https://arxiv.org/pdf/2603.29292)). Replacing "HHEM between responses" with "behavioural agreement on shared inputs" is therefore a stated contribution, not a cosmetic swap. The field has converged on execution equivalence as the clustering primitive for code. Saha et al.'s Llama3-8B result (the most perturbation-sensitive model) also points to where the signal should be strongest: small open models, the kind you can run on an 8 GB card.

## Every Perturb–Cluster–Verify ingredient already has an owner

The honest novelty assessment is **moderate-to-low on mechanism and moderate-to-high on the controlled empirical study**. The table maps each planned component to its closest prior work and to the gap that remains.

| Component | Closest prior work (headline) | What remains open |
|---|---|---|
| (1) Measurement | CodeHalu: execution-classified mapping, naming, resource and logic hallucinations; **8,883 samples, 699 APPS tasks** ([GitHub](https://github.com/yuchen814/CodeHalu)). Package hallucination: **19.7% of recommended packages**, 16 LLMs, 576k samples ([GitHub](https://github.com/Spracks/PackageHallucination)) | No composite per-model profile across automatically detectable categories, with explicit denominators, measured on contamination-resistant tasks |
| (2) Test-free detection | Paraphrase-and-execute: **75% detection at 8.6% FPR** (GPT-4) ([arXiv 2406.06864](https://arxiv.org/abs/2406.06864)); ICSE-NIER 2026 version 70.6% at 6.1% ([ACM](https://dl.acm.org/doi/10.1145/3786582.3786809)); Semantic Triangulation beats it ([arXiv 2511.12288](https://arxiv.org/abs/2511.12288)) | Character/word/sentence granularity; AUROC head-to-head against same-prompt agreement, entropy and probes at matched budget; small or quantized models |
| (3) Mitigation | Functional clustering 65%→2% error ([arXiv 2506.11021](https://arxiv.org/abs/2506.11021)); CodeRefuser abstention with distribution-free guarantee ([arXiv 2605.17029](https://arxiv.org/abs/2605.17029)); MARIN, documentation-augmented generation, MGD, PackMonitor for APIs | No published combination of a perturbed-prompt ensemble, execution clustering, an API verifier and abstention; no matched-budget proof that perturbation adds anything |
| (4) Quantization | Package hallucination rises at 8/4-bit, worst at 4-bit (Go shell commands, Qwen) ([arXiv 2512.08213](https://arxiv.org/abs/2512.08213)); 4-bit keeps correctness ([arXiv 2607.14181](https://arxiv.org/html/2607.14181v1)) | API/attribute hallucination under quantization; bit-width × perturbation × hallucination type; whether detector calibration survives quantization |

### Measurement: claim the aggregation, not the categories

Six or more taxonomies exist. CodeHalu (AAAI 2025) is the nearest competitor, because it already assigns per-model, per-category hallucination rates automatically by running code against tests ([arXiv 2405.00253](https://arxiv.org/abs/2405.00253)). A secondary review reports CodeHalu averages ranging from **GPT-4's 33.04% to Gemma-7B's 61.53%** [verify] ([liner.com](https://liner.com/review/codehalu-investigating-code-hallucinations-in-llms-via-executionbased-verification)). Zhang et al. (ISSTA 2025) hand-label eight subcategories on CoderEval and find Task Requirement Conflicts the most common ([arXiv 2409.20550](https://arxiv.org/abs/2409.20550)). HalluCode derived its taxonomy by open coding 3,120 samples ([arXiv 2404.00971](https://arxiv.org/abs/2404.00971)). Some types already have fully automatic detectors:

- An AST-plus-introspection detector reaches **100% precision and 87.6% recall** on 200 curated snippets ([arXiv 2601.19106](https://arxiv.org/abs/2601.19106)).
- Off-the-shelf static analysers catch only 14–85% of library hallucinations, which occur in **8.1–40%** of library-using tasks ([arXiv 2604.07755](https://arxiv.org/abs/2604.07755v1)).
- Registry lookups detect package hallucination ([GitHub](https://github.com/Spracks/PackageHallucination)).

No established composite "code hallucination index" was found ([arXiv 2504.20799](https://arxiv.org/abs/2504.20799)). That is the gap, but it is narrow. Reviewers will reject a new taxonomy. They will accept a decomposed profile mapped onto CodeHalu's types plus package/API existence, with a sensitivity analysis on the weights. Two more risks shape the design. First, package-hallucination rates have shrunk to **4.62–6.10% in 2026 frontier models** ([arXiv 2605.17062](https://arxiv.org/abs/2605.17062)), so a score dominated by that category compresses for strong models (local 7B models should still show spread). Second, the September 2026 "Refusing the Impossible" taxonomy explicitly separates hallucination from ordinary bugs ([arXiv 2609.03267](https://arxiv.org/html/2609.03267v1)). Your definitions must take a position on that line.

### Detection: the idea is taken, the comparison is not

Wang and Zhu's ICSE-NIER paper paraphrases each prompt five times and cross-validates the resulting programs. The 70.6% detection rate rests on only about **17 erroneous programs (12/17)** [verify] ([researchr](https://conf.researchr.org/details/icse-2026/icse-2026-nier/11/Automatic-Validation-of-LLM-Generated-Code-with-Prompt-Paraphrasing)). They also report the failure mode that matters most for your design: near-identical paraphrases reproduce the same bug and outvote correct programs ([arXiv 2406.06864](https://arxiv.org/abs/2406.06864)). Related work extends the approach:

- MRSQLGen (FSE 2026) and SQLHD (F1 69.36–82.76%) bring metamorphic prompting to text-to-SQL ([ACM](https://dl.acm.org/doi/10.1145/3797146); [arXiv 2512.22250](https://arxiv.org/html/2512.22250v1)).
- **Semantic Triangulation (OOPSLA 2026)** rewrites problems into deliberately non-equivalent "dissociative" variants and reports that it "significantly outperformed" paraphrase-based testing ([arXiv 2511.12288](https://arxiv.org/abs/2511.12288)). This is the most dangerous citation for your framing.

On the uncertainty side, 2025–2026 is crowded:

- Symbolic-execution semantic entropy ([arXiv 2502.11620](https://arxiv.org/abs/2502.11620))
- Incoherence, which flags about two-thirds of wrong programs with no reported false positives ([arXiv 2507.00057](https://arxiv.org/abs/2507.00057))
- Code-MUE, with Spearman correlation up to −0.98 ([arXiv 2607.12273](https://arxiv.org/abs/2607.12273))
- A three-axis ensemble that raises average AUROC from **0.696 to 0.776** ([arXiv 2606.09577](https://arxiv.org/html/2606.09577))

That last paper's ablation is telling. Without generated tests its AUROC falls from 0.748 to 0.617, so execution behaviour carries most of the signal. Cheap baselines are mandatory. Token confidence in code models is poorly calibrated (**ECE 0.09–0.73**) ([ICSE 2025 PDF](https://software-lab.org/publications/icse2025_calibration.pdf)). Entropy over code tokens alone reaches 0.716 AUROC, while entropy over comment tokens alone (0.375) is worse than random ([arXiv 2606.09577](https://arxiv.org/html/2606.09577)). These papers report different metrics (detection rate/FPR, error-at-abstention, Spearman, AUROC), and **none compares cross-perturbation agreement with same-prompt agreement at an equal sample budget**. That missing head-to-head is your detection contribution.

### Mitigation: the selector is MBR-exec, so the gain must come from elsewhere

The "largest execution cluster" step in Perturb–Cluster–Verify is MBR-exec and AlphaCode-style clustering, a mature baseline family:

- CodeT lifted code-davinci-002 HumanEval pass@1 **from 47.0% to 65.8%** ([arXiv 2207.10397](https://arxiv.org/pdf/2207.10397)).
- SRank adds about 3.6 points over CodeT [verify] ([ACL Anthology](https://aclanthology.org/2024.findings-acl.220/)).
- S\* lets **Qwen2.5-Coder-7B beat the 32B model by 7%** ([ACL Anthology](https://aclanthology.org/2025.findings-emnlp.865.pdf)).

The abstention layer also has owners. Thresholding the largest-cluster fraction is functional clustering, and CodeRefuser adds Learn-then-Test calibration with a distribution-free guarantee ([arXiv 2605.17029](https://arxiv.org/abs/2605.17029)). The API verifier is a post-hoc version of decode-time monitors (MGD, PackMonitor) and a retrieval-free version of De-Hallucinator ([NeurIPS 2023](https://proceedings.neurips.cc/paper_files/paper/2023/file/662b1774ba8845fc1fa3d1fc0177ceeb-Paper-Conference.pdf); [arXiv 2602.20717](https://arxiv.org/pdf/2602.20717); [arXiv 2401.01701](https://arxiv.org/html/2401.01701v3)). Working post hoc is a real advantage here: unlike constrained decoding, the check works unchanged with GGUF models and black-box APIs.

Existing evidence supports parts of the design:

- **Adaptive escalation.** CloudAPIBench shows that always retrieving documentation raises GPT-4o's valid low-frequency API calls from 38.58% to 47.94%, but can cost up to **39.02 points on high-frequency APIs**. Selective triggering yields a net +8.20% ([arXiv 2407.09726](https://arxiv.org/abs/2407.09726)).
- **Spend budget on diverse first attempts.** Under compute-matched comparison, self-repair often fails to beat i.i.d. sampling at small budgets ([ICLR 2024](https://proceedings.iclr.cc/paper_files/paper/2024/file/9ddc141bdbf9d1db510cefff56c586ad-Paper-Conference.pdf)).
- **Small models need external feedback.** For Qwen2.5-Coder 1.5–7B at Q4_K_M on a 6 GB GPU, **blind resampling beats self-repair** ([arXiv 2607.26117](https://arxiv.org/pdf/2607.26117)). A preregistered 2026 study attributes repair gains to external, execution-grounded feedback rather than self-critique ([arXiv 2606.31511](https://arxiv.org/pdf/2606.31511)).

The perturbed-prompt ensemble itself faces published counter-evidence. Random query rephrasing raised HumanEval pass@10 by **6.5–13.7%** over direct sampling ([arXiv 2502.11027](https://arxiv.org/pdf/2502.11027)). However, at a matched budget, **typo sampling stays below ordinary temperature sampling at every k** on OLMo-2-7B ([arXiv 2606.15521](https://arxiv.org/html/2606.15521v1)). Synonym and paraphrase edits corrupt prompts far less than typos ([arXiv 2506.10204](https://arxiv.org/html/2506.10204v1)). This evidence points to a specific design. Perturbation should serve mainly as an *uncertainty signal*, which is SPUQ's insight: SPUQ cut ECE by 30–70% on text tasks ([ACL Anthology](https://aclanthology.org/2024.eacl-long.143.pdf)). Character-level typos should not dominate the candidate pool, and the original prompt should keep most of the sampling budget.

### Quantization: the one place where the literature contradicts itself

The quantization evidence on accuracy is reassuring:

- 8-bit recovers **99.9%** and 4-bit **98.9%** of HumanEval accuracy ([arXiv 2411.02355](https://arxiv.org/pdf/2411.02355)).
- CodeLlama-7B goes from 29.8% at fp16 to 29.1% at 4-bit ([arXiv 2503.07103](https://arxiv.org/html/2503.07103v2)).
- Below 4 bits accuracy degrades, and CodeGemma solved **zero** tasks at 2-bit ([arXiv 2503.07103](https://arxiv.org/html/2503.07103v2)).

The evidence on hallucination and robustness points the other way, and the directions conflict:

- Package hallucination *rises* under quantization ([arXiv 2512.08213](https://arxiv.org/abs/2512.08213)).
- Quantized general LLMs are more easily misled by deceptive prompts ([arXiv 2508.19432](https://arxiv.org/html/2508.19432v1)).
- Yet quantized code models were more robust in **51.59% vs 42.86%** of adversarial experiments, a result whose metric a reviewer disputes [verify] ([arXiv 2506.22776](https://arxiv.org/html/2506.22776v1); [Pith](https://pith.science/paper/2506.22776)).
- A 2026 scaling study finds reliability peaks at 4-bit ([arXiv 2607.10855](https://arxiv.org/html/2607.10855v1)).
- The "illusion of equivalency" work shows quantized models flip many individual decisions while aggregate accuracy barely moves ([arXiv 2607.08734](https://arxiv.org/html/2607.08734)).

**No study found measures API/attribute hallucination in quantized Python models, or tests whether detector AUROC and calibration hold up as bit-width drops.** That contradiction gives the paper a clean hook.

## The publishable question is whether perturbation beats resampling at equal budget

Lead with research questions, not the method name. A working title: *"Does Perturbation Help? A Controlled Study of Test-Free Code-Hallucination Detection and Mitigation for Quantized Local LLMs."* There are four contributions, one per component.

**C1, a Code Hallucination Profile.** This is a per-model vector of automatically detected rates, each with its own denominator:

- non-existent packages or modules, per import (registry and `find_spec` checks)
- non-existent attributes or functions and signature mismatches, per call (introspection and type checking)
- undefined names, per sample
- CodeHalu-style runtime-error types, per sample
- a spec-violation proxy: passes the base tests but fails the EvalPlus extended tests, per sample
- perturbation fragility in ReCode's Robust Drop sense, per problem

A weighted aggregate is reported only as a secondary number, with a weight-sensitivity analysis. EvalPlus explicitly frames the base-to-extended pass-rate drop as a fragility measure ([GitHub](https://github.com/evalplus/evalplus)), and ReCode defines the worst-case robustness metrics ([ACL Anthology](https://aclanthology.org/2023.acl-long.773.pdf)). C1 should be presented as standardising these measures, not inventing them.

**C2, a matched-budget detection benchmark.** Score each problem's greedy output by its behavioural agreement with programs generated from perturbed prompts, separately for each perturbation level. Compare it at equal N against:

- same-prompt temperature-resampling agreement (functional clustering / incoherence style)
- code-token entropy
- static-check flags
- a hidden-state probe as a white-box upper bound on one open model ([arXiv 2501.12934](https://arxiv.org/html/2501.12934v1))

Report AUROC, AUPRC and ECE. Same-prompt resampling is the noise floor Saha et al. lacked. A related study reports that paraphrased prompts produce far more output variability than repeated identical prompts ([Springer chapter](https://link.springer.com/chapter/10.1007/978-981-95-4969-6_18)), so the gap between the two is the measurable quantity. Also report how strongly errors are correlated across perturbation levels. That speaks directly to Wang and Zhu's near-duplicate failure mode and to Semantic Triangulation's argument for non-equivalent transforms.

**C3, Perturb–Cluster–Verify as a risk–coverage method.** Stages:

1. **Stage 0 (every query).** Greedy generation, then the API-existence verifier and a code-token-entropy check.
2. **Verifier fires.** Repair with the deterministic error message: external feedback, per the evidence above.
3. **Clean code and low entropy.** Return the program.
4. **Otherwise, escalate.** Generate a perturbed-prompt ensemble that keeps the original prompt dominant, cluster programs by behaviour on EvalPlus-style mutated inputs, and return the dominant cluster when agreement exceeds a threshold calibrated with Learn-then-Test or conformal prediction on a held-out split. If agreement is too low, abstain.

Evaluate with risk–coverage curves, AURC and selective accuracy at fixed coverage. Plot pass@1 against average samples per query, compared with fixed-N MBR-exec, CodeT, SRank, functional clustering, CodeRefuser-style abstention and a reimplementation of Wang and Zhu. Charge every paraphrase-generation and repair call to the budget, following Olausson's protocol ([ICLR 2024](https://proceedings.iclr.cc/paper_files/paper/2024/file/9ddc141bdbf9d1db510cefff56c586ad-Paper-Conference.pdf)). The "same accuracy at up to 50% less compute" result for adaptive sampling is the template for the escalation claim ([ICLR 2025](https://proceedings.iclr.cc/paper_files/paper/2025/file/ff414825df833edb8b1839e3d5d495e9-Paper-Conference.pdf)). Ablate each component on its own: verifier only, perturbation only, clustering only, and escalation versus fixed N. A known trap is "spurious consistency": wrong programs agree on weak inputs, and invalid generated inputs split programs that are actually equivalent ([arXiv 2603.27098](https://arxiv.org/html/2603.27098); [arXiv 2605.17029](https://arxiv.org/html/2605.17029)). Filter invalid inputs and report input provenance.

**C4, a quantization sweep.** Run one model family across bit-widths and measure the C1 profile, the C2 AUROC/ECE and the C3 risk–coverage. Report per-problem flip rates, not just pass@1 deltas, as the illusion-of-equivalency work recommends.

**Expected outcomes.** I expect sentence-level paraphrase to carry more correctness signal than character-level typos. Typos mostly add noise and lower per-sample accuracy, consistent with the matched-budget typo-sampling result. I expect same-prompt agreement to recover most of the AUROC, with perturbation adding a small but real increment that is largest for the smallest and most heavily quantized models. Those are the models where Saha et al.'s Llama3-8B and the package-hallucination work suggest instability concentrates. I expect the API verifier to matter mainly on library-heavy tasks and barely at all on HumanEval-style puzzles.

**Negative results are publishable.** If any of these expectations fails, especially if perturbation adds nothing over resampling, the paper still holds, because it directly qualifies Wang and Zhu and Saha et al. That robustness to outcome is the main reason to frame the work as a study rather than a method.

## A one-to-two-week GPU budget on 8 GB covers the whole study

### Models and quantization

Use **Qwen2.5-Coder-Instruct at 1.5B, 3B and 7B** as the controlled family. It has a full GGUF quantization ladder that fits 8 GB. The 7B-Instruct model is reported at **88.4% HumanEval** (secondary summary, [verify]), and running at Q4_K_M it generates **28–35 tokens/s on an RTX 4060 using about 5 GB of VRAM** ([arXiv 2409.12186](https://arxiv.org/pdf/2409.12186); [bswen](https://docs.bswen.com/blog/2026-03-27-rtx-4060-token-speed-benchmark-coding/)). Add one second family for generality:

- **DeepSeek-Coder-6.7B-Instruct** (EvalPlus HE+ 71.3, MBPP+ 65.6), or
- **OpenCoder-8B-Instruct** (HE+ 77.4, MBPP+ 71.4)

Both sets of scores come from the EvalPlus leaderboard file ([results.json](https://github.com/evalplus/evalplus.github.io/blob/main/results.json)). DeepSeek-Coder's full multi-head attention needs about 2 GB of KV cache per 4k-token sequence [verify], so on 8 GB it needs short contexts or a quantized KV cache. A 2026 generalist such as Qwen3.5-4B or 9B is optional as a "current model" check, but its code scores come only from SEO-style aggregators ([QwenLM](https://github.com/QwenLM/Qwen3.5)).

Report bits per weight, not quant labels, because the labels mislead. Q4_K_M is **4.89 bpw** and Q2_K is **3.16 bpw** ([llama.cpp](https://github.com/ggml-org/llama.cpp/blob/master/tools/quantize/README.md)). The 7B Q8_0 file (about 8.1 GB) does not fit with a KV cache, so cap 7B at Q6_K, or run a small partially offloaded Q8_0 subset as the reference. FP16 is feasible for 1.5B and borderline for 3B.

### Stack

Use **llama.cpp `llama-server`** as the single engine across all bit-widths. It runs every k-quant, supports parallel slots with continuous batching, and returns per-token top-N logprobs via `n_probs`, which gives entropy features at no extra pass ([llama.cpp server](https://github.com/ggml-org/llama.cpp/blob/master/tools/server/README.md)). Avoid Ollama's OpenAI-compatible endpoint for logprobs, which reportedly drops them ([issue #16117](https://github.com/ollama/ollama/issues/16117)). EvalPlus accepts any OpenAI-compatible backend and ships a Docker executor ([GitHub](https://github.com/evalplus/evalplus)).

### Sandbox

- Run with `--network none`, a read-only root filesystem, a non-root user, and memory, CPU and PID limits.
- Check package existence through registry metadata only. **Never `pip install` a hallucinated name**, because slopsquatting turns hallucinated names into real malware ([CACM](https://cacm.acm.org/news/nonsense-and-malicious-packages-llm-hallucinations-in-code-generation/)).

### Benchmarks

HumanEval and MBPP are measurably contaminated in public pretraining corpora ([ACL 2024](https://aclanthology.org/2024.acl-long.761.pdf)), so pair them with fresher, library-heavy tasks:

- **HumanEval+ / MBPP+.** Comparability with prior work (MBPP+ v0.2.0 has 378 tasks).
- **A LiveCodeBench date window after the models' training cutoff.** Contamination control; v6 has 1,055 problems up to Apr 2025 ([GitHub](https://github.com/LiveCodeBench/LiveCodeBench)).
- **BigCodeBench-Hard (148 tasks).** Diverse library calls, where API hallucination actually occurs ([GitHub](https://github.com/bigcode-project/bigcodebench)).

HumanEval-style puzzles exercise very few third-party APIs, so they barely test the API verifier.

### Perturbations

Mirror Saha et al.'s 3×3 design: three strategies at each of the character, word and sentence levels, drawn from ReCode and NLPerturbator ([ACL Anthology](https://aclanthology.org/2023.acl-long.773.pdf); [arXiv 2406.19783](https://arxiv.org/pdf/2406.19783)). Perturb only the natural-language docstring, never the signature or the examples. Spot-check meaning preservation by hand; ReCode reported over 90% preserved.

### Compute plan

The rough throughput figures above put a 7B Q4_K_M configuration at about **6–12 hours per 10,000 generations** with 4–8 parallel slots (about 250 output tokens each). 3B runs about 2× faster and 1.5B about 3–4× faster. LiveCodeBench outputs are longer and will cost more. These figures are estimates built from blog throughput numbers, not measured runs.

| Phase | Design | Generations | Est. GPU time | Feeds |
|---|---|---|---|---|
| A. Pilot | 7B Q4_K_M; HumanEval+ (164) × (1 greedy + 9 perturbed + 10 same-prompt temperature samples) | ~3.3k | 2–4 h | LLM4Code/NIER short paper |
| B. Anchor study | 2 families × ~990 problems (HE+, MBPP+, recent LCB slice, BCB-Hard) × 20 generations | ~40k | 2–3 days | C1–C3 |
| C. Quant sweep | Qwen2.5-Coder 1.5B/3B/7B × 5–6 levels × ~990 × 10 generations | ~170k (mostly fast small models) | 3–5 days | C4 |

**Statistics.** Fix Saha et al.'s reporting gaps:

- Report every decoding parameter, the prompt template and the per-cell sample size.
- Give bootstrap confidence intervals over problems.
- Use paired tests: McNemar for pass/fail, DeLong or a paired bootstrap for AUROC.
- Report at least three seeds for sampled conditions.
- Define every aggregate explicitly.
- Release all generations and execution logs.

Execution is CPU-bound and cheap except for BigCodeBench, whose sandbox needs its library stack installed.

### Numbers to verify before citing

| Claim | Status |
|---|---|
| Saha et al. HHEM version (HHEM-2.1?) | Unverified; check the PDF and its ref [14] |
| Wang & Zhu NIER 70.6% = 12/17 programs | Snippet-derived |
| Semantic Triangulation +21% (arXiv) vs +16% (ACM) | Version-dependent |
| Sharma & David FPR <0.02%; CodeRefuser +26.5% metric | Snippet-derived; metric unconfirmed |
| CodeHalu per-model rates; 16 vs 17 models | Secondary review; internal inconsistency |
| Spracklen 576k vs 586k samples; overall 19.7% | README vs arXiv v1 differ |
| Qwen2.5-Coder HE/HE+/MBPP+ per size | Secondary or recalled |
| 2512.08213 per-bit package-hallucination rates | Full text not read |
| "Smaller = Weaker?" 51.59% / 42.86% | Metric disputed by a reviewer |

## LLM4Code in November, ARR in January, a journal for the full study

Two near-term deadlines have just closed: the NAACL 2027 ARR cycle (12 Oct 2026) and FSE 2027 (3 Oct 2026). The remaining options fall into a clear sequence ([NAACL 2027](https://www.aclweb.org/portal/content/call-main-conference-papers-naacl-2027); [FSE 2027](https://conf.researchr.org/track/fse-2027/fse-2027-papers)).

| Venue | Deadline (as of 13 Oct 2026) | Fit |
|---|---|---|
| ICSE 2027 NIER | 23 Oct 2026 AoE | Exact topical precedent (Wang & Zhu). Only if Phase A results exist this week ([researchr](https://conf.researchr.org/track/icse-2027/icse-2027-new-ideas-and-emerging-results--nier-)) |
| ACM TRUST 2027 | 31 Oct 2026 | Trust-focused; secondary fit ([CFP](https://eigtrust.acm.org/trust2027/cfp/)) |
| LLM4Code 2027 @ ICSE | ~13 Nov 2026 (tentative) | **Most realistic first paper:** pilot plus Phase B on one family ([se-deadlines](https://se-deadlines.github.io/)) |
| ICSE 2027 SRC | 13 Nov 2026 | Enrolled students only; extended abstract ([researchr](https://conf.researchr.org/track/icse-2027/icse-2027-src)) |
| ARR → ACL 2027 | 4 Jan 2027 | Full C2–C4 study with a hallucination/consistency framing; Findings is a likely outcome ([ACL 2027](https://2027.aclweb.org/calls/main/)) |
| MSR 2027 Registered Report → EMSE | Dates [verify] | Pre-register the quantization sweep; acceptance does not depend on results ([MSR 2027](https://2027.msrconf.org/track/msr-2027-registered-reports)) |
| ACL 2027 workshops (TrustNLP, Insights from Negative Results) | ~25 Apr 2027 | Fallback, or the home for a negative result ([ACL](https://www.aclweb.org/portal/content/joint-call-workshops-proposals-2027)) |
| EMSE / IST / JSS / TOSEM | Rolling | Full four-component paper. TOSEM published the code-hallucination SLR ([arXiv 2511.00776](https://arxiv.org/abs/2511.00776)) |

For an independent author, two practical constraints matter. ARR requires authors to register as reviewers ([ARR](https://github.com/acl-org/aclrollingreview/blob/main/dates.md)), while ICSE and MSR tracks do not, as far as the notes found. And the field moves monthly: at least six directly relevant papers appeared between May and September 2026. Re-run an arXiv full-text search on "paraphrase + semantic entropy + code" the week before each submission. The NIER and LLM4Code page limits and the LLM4Code date were not confirmed and need checking on researchr.

## Conclusion

The move from Saha et al. to code is more than a domain transfer. It turns an unverifiable consistency score into a testable correctness predictor, and that changes what counts as a contribution. In text, showing that models disagree under perturbation was the finding. In code, disagreement is cheap to measure and already exploited. The open scientific question is whether perturbation tells you anything resampling does not, and the matched-budget published evidence (typo sampling losing to temperature sampling) already leans against the naive version. A researcher on one 8 GB GPU is well placed to settle this, because the question is about small, quantized, locally run models. Frontier-model papers have not covered that regime, and it is where both hallucination rates and perturbation sensitivity are highest.

The practical implication is to design for every outcome. If perturbation adds signal, Perturb–Cluster–Verify becomes a cheap, model-agnostic wrapper for local coding assistants, with an abstention guarantee. If it adds nothing, the same experiments produce a clean negative result that corrects two literatures at once. Either way, the quantization sweep stands on its own: it asks whether hallucination detectors that work at 16 bits still work at 4. That question is newer than anything in the detection or mitigation components and is arguably the paper's most durable contribution.
