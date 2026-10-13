# Measuring Code Hallucination in LLMs: Taxonomies, Benchmarks, Metrics (state of the art through 2026)

> **Source-access caveat (read first).** In this research session arxiv.org, aclanthology.org, openreview.net, usenix.org, dl.acm.org, huggingface.co and Semantic Scholar were all blocked by the network proxy (HTTP 403 / DNS failure). Only GitHub (raw README files) and a web-search tool that returns summarized snippets were usable. Therefore:
> - Numbers marked **[verified: README]** were read directly from the authors' GitHub README.
> - Numbers marked **[search snippet]** come from search-engine extracts of the primary paper's abstract/PDF (generally reliable but not read in full).
> - Numbers marked **[secondary]** come from third-party summaries (liner.com, themoonlight.io, Pith) and **must be checked against the PDF before being quoted in a paper**.
> - Items marked **[prior knowledge, unverified]** are from the researcher's background knowledge of the paper and could not be re-checked this session.
> URLs given are to the primary source (usually arXiv) even when the content was obtained via search snippets.

## 1. What taxonomies of code hallucination exist, and for each: categories, detection method, dataset size, models, headline rates?

### Takeaway
There are at least six distinct taxonomies (CodeHalu, HalluCode/Liu et al., Zhang et al. ISSTA'25, CodeMirage, Lee et al. survey, plus security-focused "package/library hallucination" lines of work), and they disagree on axes: CodeHalu is the only major one whose categories are assigned **fully automatically by execution**; HalluCode and Zhang et al. rely on **manual/open-coding labels**; CodeMirage and Collu-Bench target **hallucination detection/prediction** rather than per-model rates; and the package/API line (Spracklen, CloudAPIBench, Twist, 2026 replications) gives the cleanest **per-model automatically-measured rates** but only for one category. A per-model "Code Hallucination Score" combining several automatically detectable types does not appear to exist as such, but each of its proposed components has substantial prior art.

### Cited Findings

#### CodeHalu / CodeHaluEval (Tian et al.; AAAI 2025; arXiv May 2024)
- Authors: Yuchen Tian, Weixiang Yan, Qian Yang, Xuandong Zhao, Qian Chen, Wen Wang, Ziyang Luo, Lei Ma, Dawn Song; arXiv 2405.00253 **[verified: README citation]** — [GitHub yuchen814/CodeHalu](https://github.com/yuchen814/CodeHalu); published at AAAI 2025 — [mlanthology AAAI 2025 listing](https://mlanthology.org/aaai/2025/tian2025aaai-codehalu/)
- Four primary hallucination types: **mapping, naming, resource, logic** **[search snippet]** — [arXiv 2405.00253](https://arxiv.org/abs/2405.00253)
- Subtypes **[prior knowledge, unverified]**: mapping = data-compliance / structure-access; naming = identity / external-source (e.g., undefined names, non-existent modules); resource = physical-constraint / computational-boundary (e.g., memory, recursion, timeouts); logic = logic-deviation / logic-breakdown. Tasks are derived from APPS — [arXiv 2405.00253](https://arxiv.org/abs/2405.00253)
- Detection: **execution-based verification** — generated code is run in a sandbox against test cases, and the error type/actual output is compared to the expected output to assign a hallucination type (the "CodeHalu dynamic detection algorithm") **[search snippet + README]** — [arXiv 2405.00253](https://arxiv.org/abs/2405.00253); [GitHub README](https://github.com/yuchen814/CodeHalu) (eval script takes `--halu_type`)
- Dataset: CodeHaluEval has **8,883 samples from 699 tasks** **[verified: README]** — [GitHub yuchen814/CodeHalu](https://github.com/yuchen814/CodeHalu); data on [HF Yuchen111/CodeHaluEval](https://huggingface.co/datasets/Yuchen111/CodeHaluEval)
- Models: abstract says **17** LLMs, a footnote says **16** LLMs on **105,958** code samples — internal inconsistency noted **[search snippet]** — [arXiv 2405.00253](https://arxiv.org/pdf/2405.00253)
- Syntax errors are rare: average syntax error rate **0.0020** across >100,000 samples **[search snippet]** — [arXiv 2405.00253](https://arxiv.org/pdf/2405.00253)
- Metric: Hallucination Rate per category, HR = (# detected hallucination samples / total samples) × 100% **[secondary]** — [liner.com review](https://liner.com/review/codehalu-investigating-code-hallucinations-in-llms-via-executionbased-verification)
- Per-model headline numbers **[secondary — verify]**: GPT-4 lowest average HR **33.04%**, Gemma-7B highest **61.53%**; mapping HR GPT-4 **19.19%**, GPT-3.5 **21.28%**; naming HR Claude-3-haiku **6.69%** vs LLaMA-2-7B **76.36%** — [liner.com review](https://liner.com/review/codehalu-investigating-code-hallucinations-in-llms-via-executionbased-verification); another secondary summary says average rates across models range "20% to 60%" and GPT-4 / LLaMA-3-8B were most robust at ~33% — [themoonlight.io review](https://www.themoonlight.io/en/review/codehalu-investigating-code-hallucinations-in-llms-via-execution-based-verification)

#### HalluCode — Liu et al. 2024, "Exploring and Evaluating Hallucinations in LLM-Powered Code Generation" (arXiv 2404.00971; first version titled "Beyond Functional Correctness: Exploring Hallucinations in LLM-Generated Code")
- Two versions with different taxonomies. Earlier version: **3 primary categories and 12 specific categories**, derived (by manual open coding) from **3,120 code samples** generated by different LLMs on two widely used benchmarks **[search snippet]** — [arXiv 2404.00971](https://arxiv.org/abs/2404.00971)
- Later version: "5 primary categories … depending on the conflicting objectives and varying degrees of deviation"; names given in summaries: **Intent Conflicting, Context Deviation (with subtypes context inconsistency, context repetition, dead code), Knowledge Conflicting** — the "5 vs 6 names" mismatch in summaries is unresolved **[search snippet / secondary]** — [arXiv 2404.00971](https://arxiv.org/abs/2404.00971); [themoonlight.io review](https://www.themoonlight.io/en/review/exploring-and-evaluating-hallucinations-in-llm-powered-code-generation)
- HalluCode benchmark: Python tasks annotated with hallucination types; one secondary source lists **5,663** items built on CodeAlpaca **[secondary]** — [emergentmind summary](https://www.emergentmind.com/papers/2404.00971)
- Task framed as hallucination **recognition** by LLMs (models judge whether/which hallucination is present) — i.e., labels are human/derived and evaluation is classification; one unverified secondary figure says ChatGPT reaches ~89% detection accuracy **[secondary — unverified]** — [arXiv 2404.00971](https://arxiv.org/abs/2404.00971)
- Mitigation observations: requirement refinement / chain-of-thought helps "Requirement Conflicting" hallucinations; adding domain knowledge helps "Knowledge Conflicting" ones **[search snippet]** — [arXiv 2404.00971](https://arxiv.org/pdf/2404.00971)

#### Zhang et al., "LLM Hallucinations in Practical Code Generation: Phenomena, Mechanism, and Mitigation" (Proc. ACM Softw. Eng. Vol. 2, **ISSTA 2025**, article ISSTA022, doi 10.1145/3728894; arXiv 2409.20550)
- Venue is PACMSE/ISSTA 2025 (not FSE) **[search snippet]** — [ACM DL](https://dl.acm.org/doi/10.1145/3728894); [arXiv 2409.20550](https://arxiv.org/abs/2409.20550)
- Authors from Sun Yat-sen University and NTU; six LLMs: **ChatGPT, CodeGen, PanGu-α, StarCoder2, DeepSeekCoder, CodeLlama**, on the **CoderEval** Python (repository-level) dataset **[search snippet]** — [arXiv 2409.20550](https://arxiv.org/abs/2409.20550)
- Eight subcategories: **Functional Requirement Violation, Non-Functional Requirement Violation, Background Knowledge Conflicts, Library Knowledge Conflicts, API Knowledge Conflicts, Environment Conflicts, Dependency Conflicts, Non-code Resource Conflicts** **[search snippet]** — [arXiv 2409.20550](https://arxiv.org/html/2409.20550v1). These group into three primary categories, **Task Requirement Conflicts, Factual Knowledge Conflicts, Project Context Conflicts** **[prior knowledge; consistent with the paper's statement that "Task Requirement Conflicts are the most prevalent"]** — [arXiv 2409.20550](https://arxiv.org/abs/2409.20550)
- Headline: Task Requirement Conflicts are the most prevalent type across all models (exact percentages not retrieved) **[search snippet]** — [arXiv 2409.20550](https://arxiv.org/abs/2409.20550)
- Labeling: manual (open coding) of failing generations **[prior knowledge, unverified]**.
- Root causes identified: training data quality, intention understanding, knowledge acquisition, repository-level context awareness; mitigation is a RAG approach that "works consistently across the studied models" **[search snippet]** — [arXiv 2409.20550](https://arxiv.org/abs/2409.20550)

#### CodeMirage (Agarwal et al., arXiv 2408.08333, 2024)
- Dataset: **1,137 GPT-3.5–generated hallucinated code snippets** for Python problems from **HumanEval and MBPP** **[search snippet]** — [arXiv 2408.08333](https://arxiv.org/abs/2408.08333)
- Purpose: hallucination **detection** benchmark; detectors tested include CodeLLaMA, GPT-3.5, GPT-4 (one-shot prompting) vs a fine-tuned CodeBERT baseline; GPT-4 best on HumanEval and comparable to CodeBERT on MBPP (exact F1 not retrieved) **[search snippet]** — [arXiv 2408.08333](https://arxiv.org/abs/2408.08333)
- Hallucinated snippets were deliberately **induced** by prompting GPT-3.5 to produce specific hallucination types (so labels are synthetic, LLM-generated) **[prior knowledge, unverified]**; category list (prior knowledge, unverified): syntax errors, logical errors, robustness issues, security vulnerabilities, code inefficiency — [arXiv 2408.08333](https://arxiv.org/abs/2408.08333)
- Related but distinct: "Code Hallucination" (arXiv 2407.04831), a separate 2024 paper — [arXiv 2407.04831](https://arxiv.org/html/arXiv:2407.04831)

#### Collu-Bench (lt-asset group / Purdue; arXiv 2410.09997, 2024)
- **13,234** code hallucination instances from **five datasets** and **11 LLMs** (open-source to commercial), covering **code generation and automated program repair** **[search snippet]**; HF dataset card lists **13,233** — [HF lt-asset/collu-bench](https://huggingface.co/datasets/lt-asset/collu-bench); [themoonlight.io review](https://www.themoonlight.io/en/review/collu-bench-a-benchmark-for-predicting-language-model-hallucinations-in-code)
- Framed as **predicting** hallucination (localizing the hallucinated token) from signals such as per-token log-probs and execution feedback; traditional ML and neural predictors reach only **22.03–33.15% accuracy** **[search snippet]**; best are Random Forest and LSTM (~33%) per a later paper — [CoHalLo arXiv 2512.24183](https://www.arxiv.org/pdf/2512.24183)

#### Package hallucination — Spracklen et al., "We Have a Package for You!" (USENIX Security 2025; arXiv 2406.10279)
- **16** LLMs (commercial + open-source), **Python and JavaScript**, **576,000** code samples; **19.7%** of recommended packages hallucinated; **205,474** unique hallucinated package names **[verified: README]** — [GitHub Spracks/PackageHallucination](https://github.com/Spracks/PackageHallucination); [USENIX page](https://www.usenix.org/conference/usenixsecurity25/presentation/spracklen)
- Commercial vs open-source: at least **5.2%** (commercial) vs **21.7%** (open-source) average hallucinated packages **[search snippet]** — [arXiv 2406.10279](https://arxiv.org/abs/2406.10279). Note: arXiv v1 abstract says 586,000 samples vs 576,000 elsewhere — [arXiv 2406.10279](https://arxiv.org/abs/2406.10279)
- Detection: fully automatic — extract package names (`pip install` / import / `npm install`) and check against PyPI / npm registry snapshots **[verified: README overview]** — [GitHub](https://github.com/Spracks/PackageHallucination)
- Prompt datasets: Stack Overflow–derived and LLM-generated (files SO_AT, SO_LY, LLM_AT, LLM_LY) per language **[verified: README]** — [GitHub](https://github.com/Spracks/PackageHallucination)
- Mitigations evaluated: RAG, self-detection, fine-tuning (largest reduction from fine-tuning) **[verified: README]** — [GitHub](https://github.com/Spracks/PackageHallucination); data on [Zenodo 14676377](https://zenodo.org/records/14676377)
- 2026 replication, "The Range Shrinks, the Threat Remains" (arXiv 2605.17062): five frontier models; overall package-hallucination rates **4.62% (Claude Haiku 4.5) to 6.10% (GPT-5.4-mini)**; **127** package names invented identically by all five models, **53** still registrable on PyPI/npm after disclosure **[search snippet]** — [arXiv 2605.17062](https://arxiv.org/abs/2605.17062)
- Further 2026 mitigation work on package hallucination: "LLM Ghostbusters" adaptive unlearning — [arXiv 2605.01047](https://arxiv.org/pdf/2605.01047); model editing — [arXiv 2607.02052](https://arxiv.org/pdf/2607.02052)
- "Slopsquatting" is the popular name for registering hallucinated package names (discussed in CACM news coverage) — [CACM](https://cacm.acm.org/news/nonsense-and-malicious-packages-llm-hallucinations-in-code-generation/)

#### Library hallucination — Twist et al. (arXiv 2509.22202, 2025; "Library Hallucinations in LLM-Generated Code: A Risk Analysis Grounded in Developer Queries")
- One-character misspellings in developer prompts (e.g., "numpi") trigger library hallucinations in **up to 26%** of tasks (maximum across models); 6–7 LLMs depending on version **[search snippet]** — [arXiv 2509.22202](https://arxiv.org/abs/2509.22202v1); [HTML v3](https://arxiv.org/html/2509.22202v3)
- Relevance: this is direct prior art for the "fragility to prompt perturbation" component *specifically for hallucination* (not just pass@k).

#### API hallucination — CloudAPIBench, Jain et al., "On Mitigating Code LLM Hallucinations with API Documentation" (Amazon; arXiv 2407.09726; IEEE venue per IEEE Xplore 11121691, 2025)
- CloudAPIBench: AWS and Azure API invocations annotated with API **frequency** in pretraining-like data (low/medium/high) **[search snippet]** — [arXiv 2407.09726](https://arxiv.org/abs/2407.09726); [IEEE Xplore](https://ieeexplore.ieee.org/document/11121691/)
- GPT-4o achieves only **38.58%** valid low-frequency API invocations; documentation-augmented generation (DAG) raised low-frequency to **47.94%** but with sub-optimal retrievers cut high-frequency performance by **39.02** absolute points; DAG++ (selective retrieval via index lookup + confidence thresholding) gives best results; newer abstract reports **8.20%** absolute improvement for GPT-4o **[search snippet]** — [arXiv 2407.09726](https://arxiv.org/abs/2407.09726); [Amazon Science](https://www.amazon.science/publications/on-mitigating-code-llm-hallucinations-with-api-documentation)
- Detection: automatic validity checking of API calls (and arguments) against the API specification **[prior knowledge; consistent with "valid invocations" metric]**.
- Related: hierarchical-dependency-aware API hallucination mitigation (MARIN, with an API-hallucination benchmark) — [arXiv 2505.05057](https://arxiv.org/pdf/2505.05057); web API invocation errors via RAG + constrained decoding — [arXiv 2607.05936](https://arxiv.org/pdf/2607.05936); version-evolution benchmarks LibEvoBench — [arXiv 2606.25402](https://arxiv.org/pdf/2606.25402)

#### Other 2025–2026 work found (abstract-level only)
- Static analysis for library hallucinations (arXiv 2604.07755, 2026): on NL-to-code benchmarks requiring library use, LLMs use **non-existent library features in 8.1–40%** of cases **[search snippet]**; static analysis tools detect **14–85%** of library hallucinations, the upper bound resting on manual judgments **[secondary]** — [arXiv 2604.07755](https://arxiv.org/abs/2604.07755v1); [Pith summary](https://pith.science/paper/2604.07755)
- Deterministic AST analysis (Khati, Rodriguez-Cardenas, Pantzer, Poshyvanyk; arXiv 2601.19106; 2026 IEEE/ACM conference): AST + knowledge base built by **library introspection**, no execution; on **200** curated Python snippets, **100% precision, 87.6% recall (F1 0.934)** for knowledge-conflicting hallucinations; auto-corrected **77.0% (124/161)** **[search snippet]** — [arXiv 2601.19106](https://arxiv.org/abs/2601.19106); code [WM-SEMERU/Hallucinations-in-Code](https://github.com/WM-SEMERU/Hallucinations-in-Code)
- Lee et al. survey, "Hallucination by Code Generation LLMs: Taxonomy, Benchmarks, Mitigation, and Challenges" (arXiv 2504.20799, 2025): four primary categories — **syntactic, runtime execution, functional correctness, code quality** hallucinations **[secondary]**; groups metrics into functional-correctness (pass@k) and hallucination-detection metrics (Hallucination Rate, etc.) — [arXiv 2504.20799](https://arxiv.org/abs/2504.20799); [Pith summary](https://pith.science/paper/2504.20799)
- Systematic literature review of code hallucinations (arXiv 2511.00776, 2025) — [arXiv 2511.00776](https://arxiv.org/abs/2511.00776)
- "Refusing the Impossible: A Taxonomy and Benchmark for Code Hallucination in LLMs" (arXiv 2609.03267, 2026) — title surfaced in search; contents not retrieved — [arXiv 2609.03267](https://arxiv.org/pdf/2609.03267)
- "Delulu: A Verified Multi-Lingual Benchmark for Code Hallucination Detection in Fill-in-the-Middle Tasks" (arXiv 2605.07024, 2026) — multilingual FIM; contents not retrieved — [arXiv 2605.07024](https://arxiv.org/pdf/2605.07024)
- Detector-side work: CoHalLo (hidden-layer probing for localization, arXiv 2512.24183) — [arXiv](https://www.arxiv.org/pdf/2512.24183); CodeTD (attention topology, arXiv 2609.07779) — [arXiv](https://arxiv.org/pdf/2609.07779); span-level hallucination detection over code/tool output (arXiv 2607.00895) — [arXiv](https://arxiv.org/pdf/2607.00895)
- General hallucination benchmarks with a code slice: HALoGEN (arXiv 2501.08292) includes a programming domain — [arXiv 2501.08292](https://arxiv.org/pdf/2501.08292); HalluLens (arXiv 2504.17550) — [arXiv](https://arxiv.org/html/2504.17550v1)

### Inferences
- The proposed paper's category set maps closely onto prior work: "non-existent API/library" = CodeHalu *naming/external-source* + Zhang *Library/API Knowledge Conflicts* + Spracklen/Twist/CloudAPIBench; "undefined names" = CodeHalu *naming*; "logic errors that run but fail tests" = CodeHalu *logic* (and Lee *functional correctness*); "spec/requirement violation" = Zhang *Functional/Non-Functional Requirement Violation* and Liu *Intent Conflicting*; "fragility to prompt perturbation" = ReCode (robustness) and Twist (misspelling-triggered library hallucination). Novelty should therefore be claimed for the **aggregation/scoring methodology and fully-automatic pipeline across categories**, not for the categories themselves.
- CodeHalu is the nearest competitor: it already gives per-model, per-category, execution-detected hallucination rates. The paper must explicitly differentiate (e.g., static+execution hybrid, package/API registry checks, perturbation fragility, contamination-resistant tasks, composite weighting, calibration).
- Package-hallucination rates have dropped sharply for 2026 frontier models (5.2–21.7% in 2024-era models vs 4.62–6.10% in 2026 frontier cohort), suggesting a score dominated by that category would saturate/compress for frontier models.

### Gaps
- Could not read primary PDFs; exact per-model tables for CodeHalu, HalluCode, Zhang et al. distribution percentages, CodeMirage F1 values, and Collu-Bench per-model statistics are unverified or missing.
- HalluCode's final taxonomy (5 vs 6 named categories) and its exact dataset size (5,663 is secondary) need PDF confirmation.
- CodeMirage and Collu-Bench venues (2024 arXiv; later venues unknown) could not be confirmed.
- No details retrieved for "Refusing the Impossible" or "Delulu" beyond titles.

## 2. What metrics have been proposed for an aggregate per-model hallucination score? Any composite indices?

### Takeaway
Existing work reports **per-category rates** (CodeHalu HR, package hallucination rate, valid-API-invocation rate, Robust Pass@k for perturbation) but I found **no established composite, weighted "code hallucination index" per model**; secondary commentary explicitly states there is no single hallucination rate per LLM because each benchmark measures a different failure. This is the clearest positioning opportunity, with prior art for each component metric.

### Cited Findings
- CodeHalu: per-category Hallucination Rate HR = detected/total × 100%, and an average across categories per model (e.g., GPT-4 33.04% avg) **[secondary]** — [liner.com review](https://liner.com/review/codehalu-investigating-code-hallucinations-in-llms-via-executionbased-verification) — this per-model *average* is the closest existing thing to an aggregate score.
- Package hallucination rate = fraction of recommended packages that do not exist in the registry; reported per model and overall (19.7%) — [GitHub Spracks/PackageHallucination](https://github.com/Spracks/PackageHallucination)
- Valid API invocation rate stratified by API frequency (low/medium/high) — [arXiv 2407.09726](https://arxiv.org/abs/2407.09726)
- Robustness metrics (ReCode, ACL 2023): **Robust Pass@k** (worst case over s perturbations per prompt), **Robust Drop@k** (relative drop from nominal pass@k), **Robust Relative@k** (flips in either direction); recommended **s = 5**; 30+ natural transformations over docstrings, function/variable names, code syntax, format — [ACL Anthology 2023.acl-long.773](https://aclanthology.org/2023.acl-long.773.pdf); [GitHub amazon-science/recode](https://github.com/amazon-science/recode)
- ReCode findings: models most sensitive to syntax perturbations; MBPP perturbations cause larger changes than HumanEval; secondary numbers on HumanEval RP5@1: CodeGen-16B-mono **0.195**, InCoder-6B **0.067**, GPT-J-6B **0.037** **[secondary]** — [arXiv 2212.10264](https://arxiv.org/abs/2212.10264); [liner.com](https://liner.com/review/recode-robustness-evaluation-code-generation-models)
- EvalPlus frames the pass@k drop from base to "+" tests as a fragility/rigor measure: "Less drop means more rigorousness in code generation; while a bigger drop means the generated code tends to be fragile" **[verified: README]** — [GitHub evalplus/evalplus](https://github.com/evalplus/evalplus)
- Lee et al. survey lists metric families: functional correctness (pass@k) and hallucination detection metrics (Hallucination Rate, "Validate", and others); no aggregate index reported in available summaries **[secondary]** — [Pith summary](https://pith.science/paper/2504.20799)
- HALoGEN (general LLM hallucination, includes code domain) defines model-level hallucination scores with automatic verifiers **[prior knowledge, unverified: "hallucination score", "response ratio", "utility score"]** — [arXiv 2501.08292](https://arxiv.org/pdf/2501.08292)
- Non-peer-reviewed commentary: "there is no single hallucination rate for an LLM" because each benchmark measures a different failure — [TechnBrains blog (2026)](https://www.technbrains.com/blog/ai-hallucinations-in-coding/)
- Detector-quality metrics used in the literature: accuracy (Collu-Bench 22.03–33.15%), precision/recall/F1 (AST detector 100%/87.6%/0.934) — [HF Collu-Bench](https://huggingface.co/datasets/lt-asset/collu-bench); [arXiv 2601.19106](https://arxiv.org/abs/2601.19106)

### Inferences
- A composite score must handle different denominators (per sample, per import, per API call, per perturbation set). Prior work uses all of these, so explicit normalization and weighting choices (and sensitivity analysis of weights) would be a defensible methodological contribution.
- Because CodeHalu's categories are mutually exclusive by first-failure (execution error type), a composite that counts multiple co-occurring hallucinations per sample would differ meaningfully from CodeHalu.

### Gaps
- I could not confirm whether any 2025–2026 paper (e.g., "Refusing the Impossible", Delulu, SLR 2511.00776) proposes a composite index; full texts unavailable.
- HALoGEN metric definitions need verification from the PDF.

## 3. Which categories can be detected fully automatically (no human labels), and with what tools?

### Takeaway
Non-existent packages (registry lookup), non-existent library members / undefined names (static analysis, AST + introspection), runtime-error hallucination types (execution sandbox with error-type classification), and test-failing logic errors (execution against tests) are all already detected fully automatically in published work. Requirement/spec violations and intent conflicts have so far relied on **human labels** (HalluCode, Zhang et al.) or LLM judges; perturbation fragility is automatic given semantic-preserving transformations (ReCode) and tests.

### Cited Findings
- Execution sandbox + test cases + error/output comparison classifies mapping/naming/resource/logic hallucinations automatically (CodeHalu) — [arXiv 2405.00253](https://arxiv.org/abs/2405.00253); [GitHub](https://github.com/yuchen814/CodeHalu)
- Registry lookup (PyPI/npm) of package names extracted from code/install commands (Spracklen) — [GitHub](https://github.com/Spracks/PackageHallucination)
- AST parsing + knowledge base built by **library introspection** detects API and identifier-level knowledge-conflicting hallucinations without execution (100% precision / 87.6% recall on 200 snippets) — [arXiv 2601.19106](https://arxiv.org/abs/2601.19106)
- Off-the-shelf static analysis tools detect only **14–85%** of library hallucinations (wide range; upper bound depends on manual judgment) while hallucination incidence is **8.1–40%** — [arXiv 2604.07755](https://arxiv.org/abs/2604.07755v1); [Pith](https://pith.science/paper/2604.07755)
- API validity checks against specs/documentation (CloudAPIBench "valid invocations") — [arXiv 2407.09726](https://arxiv.org/abs/2407.09726)
- Perturbation-based robustness via automatic, semantics-preserving transformations (ReCode; human eval confirmed most perturbations preserve meaning) — [ACL Anthology](https://aclanthology.org/2023.acl-long.773.pdf)
- Human/manual labeling used for: HalluCode taxonomy derivation from 3,120 samples — [arXiv 2404.00971](https://arxiv.org/abs/2404.00971); Zhang et al. CoderEval taxonomy — [arXiv 2409.20550](https://arxiv.org/abs/2409.20550)
- LLM-generated (synthetic) labels: CodeMirage's 1,137 GPT-3.5-generated hallucinated snippets — [arXiv 2408.08333](https://arxiv.org/abs/2408.08333)
- Model-internal signals (no labels at inference, but labels needed for training): token log-probs / hidden-state probes (Collu-Bench, CoHalLo, CodeTD) — [HF Collu-Bench](https://huggingface.co/datasets/lt-asset/collu-bench); [arXiv 2512.24183](https://www.arxiv.org/pdf/2512.24183); [arXiv 2609.07779](https://arxiv.org/pdf/2609.07779)

### Inferences
- Tool mapping for the proposed score (Python): pyflakes / ruff F821 for undefined names; `importlib.util.find_spec` + PyPI JSON API / offline index for non-existent modules/packages; `hasattr`/`inspect` introspection on imported modules for non-existent attributes/functions; mypy/pyright for signature/arity mismatches; sandboxed execution (e.g., EvalPlus/BigCodeBench harness) for runtime exceptions and test failures; ReCode-style transforms for fragility. None of these individual tools is novel; the combination and the per-model score are where novelty lies.
- Spec/requirement violation is the hardest to automate; a defensible fully-automatic proxy is "fails hidden/extended tests (EvalPlus+) despite passing base tests" or property-based checks, which should be explicitly framed as a proxy.
- Static-analysis false negatives (14–85% detection in 2604.07755) imply the score should report detector recall or combine static and dynamic checks.

### Gaps
- No quantitative comparison found of pyflakes vs mypy vs introspection specifically for hallucination detection beyond 2604.07755 (whose exact per-tool numbers were not retrieved).

## 4. Standard execution benchmarks with tests, and what is known about HumanEval/MBPP contamination

### Takeaway
EvalPlus (HumanEval+/MBPP+), LiveCodeBench, BigCodeBench and DS-1000 are the standard test-backed Python benchmarks; HumanEval and MBPP are demonstrably contaminated in public pretraining corpora, so a hallucination score built on them should include a contamination-resistant counterpart (LiveCodeBench date windows, BigCodeBench, perturbed/new tasks).

### Cited Findings
- **EvalPlus** (NeurIPS 2023): HumanEval+ has **80x** more tests than HumanEval; MBPP+ **35x** more tests than MBPP; MBPP+ v0.2.0 pruned broken tasks **399 → 378** (~4pp pass@1 increase expected); multiple oracle/contract fixes across versions; leaderboard at evalplus.github.io; EvalPerf (COLM 2024) for efficiency **[verified: README]** — [GitHub evalplus/evalplus](https://github.com/evalplus/evalplus)
- **LiveCodeBench** ("Holistic and Contamination Free Evaluation"): continuously collects problems from LeetCode, AtCoder, CodeForces; also self-repair, code execution, test-output prediction scenarios. Releases: v1 **400** problems (May 2023–Mar 2024), v2 **511** (to May 2024), v3 **612** (to Jul 2024), v4 **713** (to Sep 2024), v5 **880** (to Jan 2025), v6 **1055** (to Apr 2025); supports `--start_date/--end_date`; paper reports DeepSeek results only on problems after Aug 2023 to counter contamination **[verified: README]** — [GitHub LiveCodeBench](https://github.com/LiveCodeBench/LiveCodeBench)
- **BigCodeBench** (2024): **1,140** software-engineering-oriented tasks with diverse function calls; BigCodeBench-Hard **148** tasks; **139** models evaluated on Hard as of 2024-10-01 **[verified: README]** — [GitHub bigcode-project/bigcodebench](https://github.com/bigcode-project/bigcodebench); paper [arXiv 2406.15877](https://arxiv.org/abs/2406.15877)
- **DS-1000**: data-science code generation benchmark (NumPy, Pandas, etc.), simplified format on HF since 04/2024 **[verified: README]** — [GitHub xlang-ai/DS-1000](https://github.com/xlang-ai/DS-1000); 1,000 problems from 7 libraries with problem perturbations to resist memorization **[prior knowledge, unverified]** — [arXiv 2211.11501](https://arxiv.org/abs/2211.11501)
- **Contamination**: Riddell, Ni & Cohan, "Quantifying Contamination in Evaluating Code Generation" (ACL 2024): severe contamination of MBPP and HumanEval within The Pile and StarCoderData using surface (edit distance) and semantic (AST-based) matching; models perform significantly better on questions whose similar solutions were seen — [ACL Anthology 2024.acl-long.761](https://aclanthology.org/2024.acl-long.761.pdf); [arXiv 2403.04811](https://arxiv.org/abs/2403.04811)
- Follow-up: **50.8%** (MBPP) and **63.4%** (HumanEval) of samples have a Stack code sample with >80 similarity to ground truth **[search snippet; source paper attribution uncertain]** — [arXiv 2410.03837 / related](https://arxiv.org/pdf/2410.03837)
- **65.4%** of MBPP test instances flagged as contaminated by a leakage-detection tool (from benchmark-quality study) **[search snippet]** — [arXiv 2404.10155 "The Fault in our Stars"](https://arxiv.org/pdf/2404.10155)
- LBPP / "On Leakage of Code Generation Evaluation Datasets" argues contamination is likely unavoidable at LLM scale — [arXiv 2407.07565](https://arxiv.org/pdf/2407.07565)
- 2026 fine-grained contamination detection for code LLMs: TRACER — [arXiv 2605.24079](https://arxiv.org/pdf/2605.24079)
- Benchmark-quality issues (broken tasks, flawed oracles) are themselves documented: EvalPlus fix log — [GitHub evalplus](https://github.com/evalplus/evalplus); "The Fault in our Stars" — [arXiv 2404.10155](https://arxiv.org/pdf/2404.10155)

### Inferences
- CodeHalu draws from APPS and CodeMirage from HumanEval/MBPP — both contamination-exposed; a contamination-aware hallucination score (e.g., computed on LiveCodeBench post-cutoff windows and BigCodeBench) would be a differentiator.
- BigCodeBench's heavy library-call content makes it the natural host for API/library hallucination measurement with tests.

### Gaps
- Exact attribution of the 50.8%/63.4% figure not confirmed.
- No direct evidence found on contamination of BigCodeBench or DS-1000 by 2026-era models.

## 5. Limitations/gaps listed by the papers themselves

### Takeaway
Self-reported limitations cluster around: Python-only or narrow-language scope, manual labeling cost/subjectivity, synthetic (LLM-induced) hallucinations, weak detectors, benchmark realism, and taxonomies that do not map onto benchmarks.

### Cited Findings
- Lee et al. survey: current benchmarks lack language diversity and fail to reflect real-world workloads; prior surveys did not systematically explore code hallucinations — [arXiv 2504.20799](https://arxiv.org/abs/2504.20799) (via [themoonlight.io](https://www.themoonlight.io/en/review/hallucination-by-code-generation-llms-taxonomy-benchmarks-mitigation-and-challenges)); critique that its categories overlap and benchmarks do not map onto the taxonomy (one reviewer's opinion) — [Pith](https://pith.science/paper/2504.20799)
- Collu-Bench: best predictors only ~22–33% accurate, i.e., hallucination prediction remains hard — [HF dataset](https://huggingface.co/datasets/lt-asset/collu-bench); [CoHalLo](https://www.arxiv.org/pdf/2512.24183)
- AST detector (2601.19106): single curated 200-snippet dataset; generalization unclear — [arXiv 2601.19106](https://arxiv.org/abs/2601.19106)
- Static analysis study (2604.07755): detection upper bounds rest on manual judgments not fully documented **[secondary]** — [Pith](https://pith.science/paper/2604.07755)
- CloudAPIBench: retrieval helps rare APIs but can hurt frequent APIs with sub-optimal retrievers (-39.02 points) — [arXiv 2407.09726](https://arxiv.org/abs/2407.09726)
- Package hallucination 2026 replication: rates compress across frontier models but shared hallucinated names persist (127 shared; 53 registrable) — [arXiv 2605.17062](https://arxiv.org/abs/2605.17062)
- CodeHalu internal inconsistency in model count (16 vs 17) — [arXiv 2405.00253](https://arxiv.org/pdf/2405.00253)
- Multilingual coverage is sparse: Spracklen (Python + JavaScript) — [GitHub](https://github.com/Spracks/PackageHallucination); Delulu (multi-lingual FIM) — [arXiv 2605.07024](https://arxiv.org/pdf/2605.07024); CloudAPIBench (cloud SDKs) — [arXiv 2407.09726](https://arxiv.org/abs/2407.09726)

### Inferences
- A paper proposing a Code Hallucination Score can position against: (a) human-labeled taxonomies (not scalable), (b) single-category automatic studies (packages/APIs only), (c) CodeHalu (automatic but execution-only, first-failure, APPS-based, possibly contaminated), (d) detection/prediction benchmarks (CodeMirage, Collu-Bench) which evaluate detectors not generators.
- Must avoid claiming novelty for: the hallucination categories themselves; execution-based hallucination classification (CodeHalu); package/registry checking (Spracklen); introspection-based API checks (2601.19106); perturbation robustness metrics (ReCode Robust Pass@k); extended-test fragility (EvalPlus).

### Gaps
- Explicit "Limitations" sections of CodeHalu, HalluCode, Zhang et al., CodeMirage could not be read (arXiv blocked); the report writer should verify these before citing specific limitations.
