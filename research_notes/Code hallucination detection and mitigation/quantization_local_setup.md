# Quantization effects on code LLMs (correctness / hallucination / robustness) and a practical 8 GB-GPU experiment setup (state as of Oct 2026)

> Research-process caveat: in this environment arxiv.org, huggingface.co, alphaxiv, ollama.com, docs.vllm.ai and the EvalPlus/LiveCodeBench github.io sites were **unreachable** (proxy-denied). Paper findings below come from search-engine extracts of arXiv abstracts/HTML (URLs given), not from reading full PDFs, so exact per-table numbers are only quoted where they surfaced verbatim. GitHub raw content *was* reachable, so tool/harness facts (llama.cpp, vLLM, EvalPlus, bigcode-harness, LiveCodeBench, Qwen3.5 README, EvalPlus leaderboard `results.json`) are from primary sources.

## Q1. What is known about how quantization (8/6/4/3/2-bit; GGUF k-quants, GPTQ, AWQ, bitsandbytes) affects code-generation accuracy, hallucination, trustworthiness and robustness?

### Takeaway
Across 2024–2026 studies the consensus is: 8-bit is essentially lossless and 4-bit (GPTQ/AWQ/GGUF Q4_K_M-class) usually keeps ~97–99% of functional correctness on HumanEval-style benchmarks, while 3-bit starts to hurt and 2-bit collapses (sometimes to zero) for ~7B code models. But aggregate pass@1 hides per-item churn ("illusion of equivalency"), trustworthiness drops noticeably at 3-bit, and the one code-specific hallucination study found 4-bit models hallucinate **more** non-existent packages — so "4-bit is fine" holds for accuracy, not necessarily for hallucination.

### Cited Findings

**General-LLM trust / hallucination studies (2024–2025)**
- Hong et al., "Decoding Compressed Trust" (ICML 2024): 3 × 13B models (LLaMA-2 13B, LLaMA-2 13B Chat, Vicuna-13B), 5 compression methods, 8 trust dimensions (stereotype, toxicity, privacy, fairness, ethics, adversarial/OOD robustness, adversarial demonstrations). 4-bit quantized models retained the trustworthiness of the dense original; pruning degraded trust even at 50% sparsity; moderate quantization *improved* ethics and fairness; at 3-bit trustworthiness dropped considerably; benign accuracy alone hides these risks. (Older: 2024, Llama-2 era; no code tasks.) — [arXiv 2403.15447](https://arxiv.org/abs/2403.15447v1); [ICML 2024 poster](https://icml.cc/virtual/2024/poster/33520)
- Li et al., "Evaluating Quantized Large Language Models" (ICML 2024, PMLR 235:28480–28524): PTQ of weights, activations and KV cache over 11 families (OPT, LLaMA2, Falcon, Bloomz, Mistral, ChatGLM, Vicuna, LongChat, StableLM, Gemma, Mamba; 125M–180B) on basic NLP, emergent abilities, trustworthiness, dialogue and long-context; code at github.com/thu-nics/qllm-eval. (Did not surface code-generation-specific numbers; older 2024.) — [PMLR](https://proceedings.mlr.press/v235/li24bb.html); [arXiv 2402.18158](https://arxiv.org/pdf/2402.18158)
- Kurtic et al., "Give Me BF16 or Give Me Death?" (Neural Magic; arXiv Nov 2024): on HumanEval and HumanEval+, 8-bit models recovered 99.9% of baseline accuracy and 4-bit models 98.9%. — [arXiv 2411.02355](https://arxiv.org/pdf/2411.02355)
- Fu et al., "Quantized but Deceptive? A Multi-Dimensional Truthfulness Evaluation of Quantized LLMs" (EMNLP 2025): quantized models keep internally truthful representations (layer-wise probing/PCA) but are **more easily pushed into false answers by misleading prompts**; 15 "honest/neutral/deceptive" prompt rephrasings each; 16→4-bit (AWQ, GPTQ) did not significantly hurt true/false tasks; 2-bit AQLM (LLaMA-3.1-8B, 1×16 codebook) lost up to 40% truthfulness. — [arXiv 2508.19432](https://arxiv.org/html/2508.19432v1); [ACL Anthology](https://preview.aclanthology.org/credits/2025.emnlp-main.1548)
- IJCAI 2025 paper on quantization methods/task difficulty/model size (1B–405B instruction-tuned): quantized larger models generally beat smaller FP16 baselines but often struggle with instruction-following and hallucination detection; on TruthfulQA the unquantized Llama-2-7B was better at hallucination detection. — [IJCAI 2025 proceedings PDF](https://www.ijcai.org/proceedings/2025/0902.pdf)
- Ayadi, Daróczi, Günnemann, Charpentier, "Reliability Scaling Laws for Quantized LLMs" (arXiv Jul 2026; ICLR 2026 submission): performance scales monotonically with total bits but **reliability scales non-linearly, peaking at 4-bit**; quantization can *enhance* robustness to natural character/word-level perturbations (typos, slang, emoji, leetspeak). General-purpose, not code. — [arXiv 2607.10855](https://arxiv.org/html/2607.10855v1); [OpenReview](https://openreview.net/forum?id=QhkW8xPH1v)
- Rababah et al., "The Illusion of Equivalency" (arXiv Jul 2026): introduces "correctness agreement"; quantized LLMs can lose many of the base model's correct decisions while accuracy/perplexity stay nearly unchanged; agreement declines sharply below 4 bits; Q/K projections more quantization-sensitive than V/O. — [arXiv 2607.08734](https://arxiv.org/html/2607.08734); [Emergent Mind summary](https://www.emergentmind.com/papers/2607.08734)
- Follow-up "Which Decisions Low-Bit Quantization Breaks, and How to Predict Them": quantizing only the KV cache (W8A4 setting) flips 3–18% of decisions on measured models. — [arXiv 2608.06564](https://arxiv.org/html/2608.06564)

**Code-specific studies (2024–2026)**
- Nyamsuren, "Evaluating Quantized LLMs for Code Generation on Low-Resource Language Benchmarks" (Oct 2024): five ~7B code models (DeepSeek-Coder-6.7B-Instruct, CodeQwen1.5-7B-Chat, CodeLlama-7B-Instruct, StarCoder2-7B, + one more) at 2/4/8-bit on Lua (MultiPL-E HumanEval/MBPP, MCEVAL) on a CPU-only laptop; 4-bit is the best trade-off; performance drops significantly at 2-bit; DeepSeek-Coder barely changes 4→8-bit, CodeQwen gains ~7 pp at 8-bit; pass@1 mostly <50% on Lua; longer inference time doesn't compensate for lower precision. — [arXiv 2410.14766](https://arxiv.org/html/2410.14766v1); [HackerNoon summary](https://hackernoon.com/why-4-bit-quantization-is-the-sweet-spot-for-code-llms)
- Giagnorio, Mastropaolo et al., "Evaluating the Impact of Post-Training Quantization on LLMs for Code Generation" (v1 title "Quantizing LLMs for Code Generation: A Differentiated Replication", 2025): CodeLlama & DeepSeek-Coder up to 34B, 2–8 bit, Python+Java (MultiPL-E, McEval); **4-bit is the new frontier: ~70% memory reduction without significant drop**. Example: CodeLlama-7B fp16 29.8% pass@1 (Python MultiPL-E, 13.48 GB) vs 4-bit 29.1% (4.00 GB). CodeLlama-34B at **2-bit** beats fp16 7B on Python (37.1 vs 29.8) and Java (32.7 vs 32.2) using ~9.38 GB. Code-specific calibration data helps at 3/2-bit; no significant calibration effect at 8/4-bit. Also reports CodeGemma solved **zero** tasks at 2-bit. — [arXiv 2503.07103](https://arxiv.org/html/2503.07103v2); [ACM DL](https://dl.acm.org/doi/10.1145/3794763.3794810)
- Afrin, Xu, Mastropaolo, "Is Quantization a Deal-breaker? / Quantization Is Not a Dealbreaker" (ICSME 2025): AWQ 4-bit on CodeLlama & DeepSeek-Coder, Java+Python; functional correctness preserved; maintainability and structural simplicity (cyclomatic/cognitive complexity, LOC) preserved. — [arXiv 2507.09665](https://arxiv.org/html/2507.09665v1); [ICSME 2025](https://conf.researchr.org/details/icsme-2025/icsme-2025-papers/32/Quantization-Is-Not-a-Dealbreaker-Empirical-Insights-from-Large-Code-Models)
- Afrin, Haque, Mastropaolo, "Quantize with Confidence? An Empirical Study of Quantization for Code Generation" (Jul 2026): six PTQ methods (GPTQ, AWQ, QuIP#, AQLM, bitsandbytes, **GGUF**) on Qwen2.5-Coder and CodeLlama, McEval + CoderEval (Python, Java); AQLM matches/exceeds full precision, **QuIP# has the largest correctness degradation, especially on complex prompts**; AWQ significantly increases code smells on McEval-Java; security attributes stable; robustness vs prompt complexity measured by Shannon entropy and token length. A third-party (machine-generated) review flags mixed checkpoint provenance and an overstated "AQLM always wins" claim. — [arXiv 2607.14181](https://arxiv.org/html/2607.14181v1); [Pith review](https://pith.science/paper/2607.14181)
- "Precision or Peril: A PoC of Python Code Quality from Quantized LLMs" (Nov 2024): 8-bit and 4-bit; effects on code quality inconsistent across models; recurring quality/maintainability issues. — [arXiv 2411.10656](https://arxiv.org/html/2411.10656v1) — contradicts the positive maintainability conclusion of [2507.09665](https://arxiv.org/html/2507.09665v1) for some settings.
- "On the Quantization Robustness of Diffusion Language Models in Coding Benchmarks" (Apr 2026): under GPTQ, autoregressive **Qwen3 lost up to 23.2 points on HumanEval-Instruct at 4-bit**, while diffusion model CoDA lost ≤1.8 points on HumanEval+; CoDA degrades more gracefully at 2–4 bits. (Notable counter-example to "4-bit is ~lossless" — depends on method/model.) — [arXiv 2604.20079](https://arxiv.org/html/2604.20079v1)
- Vallecillos-Ruiz et al., "Smaller Models, Unexpected Costs: Trade-offs in LLM Quantization for Automated Program Repair" (Jun 2026): 2-/3-bit often hurt; 4-bit sometimes matched or beat 8-bit; HQQ 3-bit (model weights) raised energy up to +1524.46% for DeepSeek-33B on HumanEval; quanto-4 (weights or KV) and HQQ-4/8 (KV) cheapest in energy; energy tracks inference time. — [arXiv 2606.27205](https://arxiv.org/pdf/2606.27205)
- Kusama et al., "How Small is Enough?" (Aug 2025): quantized small LMs on QuixBugs APR; int8 had minimal effect on accuracy while cutting memory. — [arXiv 2508.16499](https://arxiv.org/html/2508.16499v1)
- Energy/accuracy study of LMs in SE tasks uses GGUF as "most energy-efficient format" (citing Rajput & Sharma 2024). — [arXiv 2412.00329](https://arxiv.org/html/2412.00329v2)
- SmoothQuant+ (Dec 2023, older): 4-bit weight-only PTQ evaluated on Code Llama/HumanEval vs FP16, RTN, AWQ. — [arXiv 2312.03788](https://arxiv.org/pdf/2312.03788)

**GGUF k-quant fidelity reference (llama.cpp, Llama-3.1-8B)** — bits/weight and size: Q2_K 3.16 bpw / 2.95 GiB; Q3_K_M 4.00 / 3.74 GiB; Q4_K_M 4.89 / 4.58 GiB; Q5_K_M 5.70 / 5.33 GiB; Q6_K 6.56 / 6.14 GiB; Q8_0 8.50 / 7.95 GiB; F16 16.0 / 14.96 GiB. — [llama.cpp tools/quantize/README.md](https://github.com/ggml-org/llama.cpp/blob/master/tools/quantize/README.md)

### Inferences
- "Bit-width" labels are not comparable across formats: GGUF "Q4_K_M" is ~4.9 bpw and "Q2_K" ~3.2 bpw (because of mixed-precision tensors), so a GGUF Q2_K is closer to "3-bit" in GPTQ terms. Experiment design should report bpw, not just the label.
- Ordering of expected degradation for a 7B code model: Q8_0 ≈ FP16 > Q6_K ≈ Q5_K_M > Q4_K_M (≈1–3 pp pass@1 loss typical, occasionally much larger e.g. Qwen3 GPTQ case) > Q3_K_M (noticeable) > Q2_K/IQ2 (severe, sometimes total collapse). Smaller models (1.5B/3B) are likely more fragile per bit, consistent with "Smaller = Weaker?" finding mixed effects by size.
- The "illusion of equivalency" result implies that a study of hallucination under quantization should measure per-problem flip rates (pass→fail and fail→pass) and per-sample hallucination changes, not only pass@1 deltas.

### Gaps
- Could not read full tables of 2503.07103, 2410.14766, 2606.27205 or 2604.20079 (arXiv blocked), so exact per-bit numbers for DeepSeek-Coder/Qwen-Coder at 3- and 2-bit are missing.
- "Do Emergent Abilities Exist in Quantized LLMs?" (Liu et al., 2023, arXiv 2307.08072) did not surface in searches; from recollection (unverified) it found 4-bit preserves in-context learning/CoT/instruction-following and 2-bit fails — treat as unconfirmed and old (2023).
- Neither Hong et al. nor Li et al. (2024) appear to include code generation; no study found that applies the DecodingTrust-style multi-dimension trust battery to code models.

## Q2. Has anyone studied whether quantization increases sensitivity to prompt perturbations or specific hallucination types (API/package hallucination) in code models? Where exactly is the gap?

### Takeaway
Only partially. One 2025 paper (Fang et al.) tests adversarial prompt attacks on quantized code LLMs but measures pass@1 robustness, not hallucination; one Dec 2025 paper (Haque et al.) measures **package** hallucination under 8/4-bit for Go shell commands and finds quantization substantially increases it; 2026 work examines robustness vs prompt complexity. No study (found) measures **API/attribute-level hallucination** in Python/Java code under quantization, nor combines semantic-preserving prompt perturbations (ReCode-style) × hallucination taxonomy × uncertainty (logprob) signals on quantized code models.

### Cited Findings
- Fang, Ding, Mastropaolo, Xu, "Smaller = Weaker? Benchmarking Robustness of Quantized LLMs in Code Generation" (Jun 2025): four model families, 350M–33B; adversarial attacks on input prompts plus noise perturbation of weights; quantized LLMs were more robust in **51.59% vs 42.86%** of adversarial experiments; quantized models withstood broader weight-noise ranges; small models (1–3B) benefited more from quantization, >7B mixed. Hallucination not measured. — [arXiv 2506.22776](https://arxiv.org/html/2506.22776v1)
- Critique: a machine-generated review argues the headline relies on a robustness score (RRS) not normalized for baseline pass@1, so "quantization improves robustness" may be an artifact. — [Pith review](https://pith.science/paper/2506.22776) (secondary, unverified)
- Haque, Lin, Arkoh, Tadesse, Xu, "Secure or Suspect? Investigating Package Hallucinations of Shell Command in Original and Quantized LLMs" (arXiv 9 Dec 2025): claims first systematic study of quantization × package hallucination; five Qwen sizes at full precision, 8-bit and 4-bit; datasets SO, MBPP and a paraphrase set; **quantization substantially raised package hallucination rate, 4-bit worst**; among correctly generated packages, vulnerability presence rate rises as precision falls; fabricated packages mostly look like realistic but nonexistent GitHub/golang.org module paths. (Go-language, shell-command context.) — [arXiv 2512.08213](https://arxiv.org/abs/2512.08213)
- Afrin et al. 2026 analyze robustness against prompt complexity (Shannon entropy, token length) across six PTQ methods; sensitivity varies by technique; QuIP# worst on complex prompts. — [arXiv 2607.14181](https://arxiv.org/html/2607.14181v1)
- Fu et al. (EMNLP 2025, general LLMs): quantized models more easily misled by deceptive prompt framing. — [arXiv 2508.19432](https://arxiv.org/html/2508.19432v1)
- Reliability scaling laws (2026, general): quantization can increase robustness to natural input perturbations, reliability peaks at 4-bit. — [arXiv 2607.10855](https://arxiv.org/html/2607.10855v1)
- ReCode (Wang et al. 2023, older) defines 28 semantics-preserving prompt perturbations (docstrings, function names, syntax, formatting) — the standard tool such a study would reuse; it was not established that "Smaller = Weaker?" used ReCode. — [search summary referencing ReCode](https://arxiv.org/html/2506.22776v1)
- Related package-hallucination literature (non-quantized): "Importing Phantoms" (2025) — [arXiv 2501.19012](https://arxiv.org/pdf/2501.19012); inference-time defenses against package hallucination (2026) — [arXiv 2608.22652](https://arxiv.org/html/2608.22652)

### Inferences
- Precise gap statement: (1) No study of **API/function/attribute hallucination** (e.g., calling non-existent library functions, wrong signatures) in quantized Python code models; (2) no study crossing **bit-width × semantic-preserving prompt perturbation × hallucination type**; the existing perturbation study measures pass@1 only and its robustness metric is disputed; (3) no study of whether **logprob/entropy-based hallucination detectors stay calibrated** after quantization for code (Illusion-of-Equivalency suggests per-item behavior shifts even when accuracy doesn't); (4) evidence is directionally conflicting — robustness to perturbation may *improve* with 4-bit (2506.22776, 2607.10855) while hallucination of packages *worsens* (2512.08213) and susceptibility to misleading prompts worsens (2508.19432). This conflict is itself a research hook.
- A small 8 GB-GPU study (e.g., Qwen2.5-Coder-1.5B/3B/7B × Q8/Q6/Q5/Q4/Q3/Q2 × ReCode perturbations × HumanEval+/MBPP+ with static API-existence checks and token-logprob features) would be novel relative to the papers found.

### Gaps
- Full text of 2512.08213 not accessible: exact package-hallucination rates per bit-width unknown.
- Whether 2506.22776 used ReCode or another attack suite could not be confirmed.

## Q3. Best open code models that fit 8 GB VRAM (2025–2026): scores and GGUF sizes per quant

### Takeaway
For controlled quantization research, **Qwen2.5-Coder (1.5B/3B/7B-Instruct)** remains the best-documented family that fits 8 GB at Q8 (1.5B/3B) and Q4–Q6 (7B), with a full ladder of GGUF quants. Newer generalist small models (Qwen3.5-9B/4B, released 2026-03-02; Gemma 4 E2B/E4B) score higher on LiveCodeBench per community guides but have thinking modes and fewer independent code-benchmark numbers. Legacy baselines (DeepSeek-Coder-6.7B, CodeLlama-7B, StarCoder2-7B) are weaker and older (2023–2024).

### Cited Findings
**EvalPlus leaderboard (results.json in the evalplus.github.io repo; HE / HE+ / MBPP / MBPP+ pass@1, greedy)** — [EvalPlus results.json](https://github.com/evalplus/evalplus.github.io/blob/main/results.json)
- OpenCoder-8B-Instruct: 81.7 / 77.4 / 82.0 / 71.4
- CodeQwen1.5-7B-Chat: 83.5 / 78.7 / 79.4 / 69.0
- DeepSeek-Coder-6.7B-instruct: 74.4 / 71.3 / 74.9 / 65.6
- Llama3.1-8B-instruct: 69.5 / 62.8 / 68.3 / 55.6
- Phi-3-mini-4k-instruct (3.8B): 64.6 / 59.1 / 65.9 / 54.2
- codegemma-7b-it: 60.4 / 51.8 / 70.4 / 56.9
- CodeLlama-7B (base): 37.8 / 35.4 / 59.5 / 46.8
- StarCoder2-7B (base): 35.4 / 29.9 / – / –
- Qwen2.5-Coder-32B-Instruct (reference, doesn't fit 8 GB): 92.1 / 87.2 / 90.5 / 77.0
- (Qwen2.5-Coder-7B/3B/1.5B entries are not in this leaderboard file.)

**Qwen2.5-Coder** — 7B base HumanEval 61.6, MBPP 76.9; 7B-Instruct HumanEval 88.4, MBPP 83.5 (secondary summary of the tech report). — [Qwen2.5-Coder Tech Report arXiv 2409.12186](https://arxiv.org/pdf/2409.12186); [Emergent Mind summary](https://www.emergentmind.com/topics/qwen2-5-coder-7b)

**Newer (2026) small models**
- Qwen3.5-9B, 4B, 2B, 0.8B released 2026-03-02 (Qwen3.5-27B, 35B-A3B, 122B-A10B on 2026-02-24). — [QwenLM/Qwen3.5 README](https://github.com/QwenLM/Qwen3.5)
- Community guides: Qwen3.5-9B LiveCodeBench v6 = 65.6 and fits 8 GB at Q4_K_M; Qwen3.5-4B LCB v6 = 55.8; Gemma 4 E4B LCB v6 = 52.0 vs Gemma 4 12B 72.0; Gemma 4 E2B QAT build ~4.3 GB; Qwen2.5-Coder-7B "88.4% HumanEval at ~5 GB Q4" recommended for autocomplete. (SEO-style secondary aggregators with inconsistent numbers—treat as indicative.) — [llmconfigurator guide](https://llmconfigurator.com/en/guides/coding-agents/best-local-coding-models); [insiderllm guide](https://insiderllm.com/guides/best-local-coding-models-2026/); [zimaspace guide](https://shop.zimaspace.com/blogs/tech-ai-hub/best-local-coding-models-8gb-12gb-16gb-24gb-vram); [morphllm (Sep 2026)](https://www.morphllm.com/best-ollama-models)
- Qwen3-Coder family = 480B-A35B, 30B-A3B, and Qwen3-Coder-Next (built on Qwen3-Next-80B-A3B) — none fit fully in 8 GB VRAM. — [QwenLM/Qwen3-Coder README](https://github.com/QwenLM/Qwen3-Coder)

**GGUF sizes per quant** — llama.cpp reference bpw (Llama-3.1-8B): Q8_0 8.50, Q6_K 6.56, Q5_K_M 5.70, Q4_K_M 4.89, Q3_K_M 4.00, Q2_K 3.16 bpw; Llama-3.1-8B Q4_K_M = 4.9 GB vs 32.1 GB original. — [llama.cpp quantize README](https://github.com/ggml-org/llama.cpp/blob/master/tools/quantize/README.md)

### Inferences
- Approximate GGUF file sizes, computed as params × bpw/8 using the llama.cpp bpw figures (actual files differ by ±5–10% due to embedding/vocab tensors; parameter counts from model cards recalled, not fetched):

| Model (params) | Q8_0 | Q6_K | Q5_K_M | Q4_K_M | Q3_K_M | Q2_K |
|---|---|---|---|---|---|---|
| Qwen2.5-Coder-7B (7.62B) | ~8.1 GB (won't fit w/ KV) | ~6.3 | ~5.4 | ~4.7 | ~3.8 | ~3.0 |
| DeepSeek-Coder-6.7B / CodeLlama-7B (6.74B) | ~7.2 (marginal) | ~5.5 | ~4.8 | ~4.1 | ~3.4 | ~2.7 |
| StarCoder2-7B (7.17B) | ~7.6 | ~5.9 | ~5.1 | ~4.4 | ~3.6 | ~2.8 |
| ~9B (e.g., Qwen3.5-9B) | ~9.6 (no) | ~7.4 (tight) | ~6.4 | ~5.5 | ~4.5 | ~3.6 |
| Qwen2.5-Coder-3B (3.09B) | ~3.3 | ~2.5 | ~2.2 | ~1.9 | ~1.5 | ~1.2 |
| Qwen2.5-Coder-1.5B (1.54B) | ~1.6 | ~1.3 | ~1.1 | ~0.9 | ~0.8 | ~0.6 |

- Add KV cache + ~0.3–0.8 GB CUDA/compute buffers. KV-cache cost differs hugely by architecture: GQA models like Qwen2.5-7B (28 layers × 4 KV heads × 128 dim) need ~56 KB/token at FP16 (~0.23 GB per 4k-token sequence), while older full-MHA models like DeepSeek-Coder-6.7B/CodeLlama-7B (32 layers × 4096) need ~0.5 MB/token (~2 GB per 4k sequence) — so on 8 GB, MHA models must use short contexts / few parallel slots or a quantized KV cache. (Architecture figures from memory of model configs; verify.)
- Practical fit on 8 GB: full Q8→Q2 ladders for 1.5B and 3B; Q6_K→Q2_K for 7B (Q8_0 7B only with tiny context or partial offload). A cross-size × cross-bit design (1.5B/3B/7B × 5–6 quant levels) of a single family (Qwen2.5-Coder) is the cleanest fit.

### Gaps
- Could not verify Qwen2.5-Coder-7B/3B/1.5B-Instruct HumanEval+/MBPP+/LiveCodeBench numbers from the primary tech report (arXiv blocked). Recalled-but-UNVERIFIED values: 7B-Instruct HE+ ≈ 84.1, MBPP+ ≈ 71.7; 3B-Instruct HE ≈ 84.1, HE+ ≈ 80.5; 1.5B-Instruct HE ≈ 70.7, HE+ ≈ 66.5 — must be checked against arXiv 2409.12186 Table(s) before use.
- No primary HumanEval/EvalPlus numbers found for Qwen3.5-9B/4B, Gemma 4, Phi-4-mini, Llama 3.2/3.3 small models, DeepSeek-Coder-V2-Lite (16B MoE — needs CPU offload on 8 GB).
- Exact bartowski/Qwen-official GGUF file sizes could not be fetched (huggingface.co blocked).

## Q4. Inference stacks: which expose token log-probabilities, and typical tokens/sec for 7–8B Q4 on 8 GB cards?

### Takeaway
llama.cpp `llama-server` is the best fit: full GGUF k-quant ladder, continuous batching with parallel slots, and per-token top-N logprobs (native `n_probs` and OpenAI-compatible `logprobs`). vLLM gives logprobs and high batched throughput with AWQ/GPTQ (Turing+), but an 8 GB card leaves little KV room. Ollama added logprobs in v0.12.11 (late 2025), but its OpenAI-compatible endpoint reportedly drops them. Expect ~28–40 tok/s single-stream for 7B Q4_K_M on RTX 4060-class cards.

### Cited Findings
- **llama.cpp server**: parallel decoding with multi-user support; `-np, --parallel N` slots (default auto); continuous batching enabled by default; `--kv-unified` shared KV pool; `/slots` monitoring endpoint. — [llama.cpp server README](https://github.com/ggml-org/llama.cpp/blob/master/tools/server/README.md)
- **llama.cpp logprobs**: `n_probs` >0 returns top-N token probabilities per generated token; for temperature <0 tokens are sampled greedily but probabilities still computed via softmax of logits; `post_sampling_probs` returns probs after the sampler chain; response `completion_probabilities` with `logprob` and `top_logprobs` per token. — [llama.cpp server README](https://github.com/ggml-org/llama.cpp/blob/master/tools/server/README.md); OpenAI-compatible logprobs fix (#10783) — [commit mirror](https://cdn04132025.gitlink.org.cn/replica/llama.cpp/commit/57bb2c40cd94c5a09f5210ed8264cc93b21c4b7e)
- **Ollama**: v0.12.11 added log probabilities to the native API and OpenAI-compatible API (`logprobs: true`, `top_logprobs` e.g. 3; returns token, logprob, bytes). — [Ollama v0.12.11 release](https://github.com/ollama/ollama/releases/tag/v0.12.11); Go API `ChatRequest` has `Logprobs`, `TopLogprobs` (0–20). — [pkg.go.dev ollama/api](https://pkg.go.dev/github.com/ollama/ollama/api). Conflict: issue #16117 reports `/v1/chat/completions` silently drops `logprobs`/`top_logprobs`. — [ollama issue #16117](https://github.com/ollama/ollama/issues/16117)
- **vLLM**: `SamplingParams.logprobs` (top-k per output token), `prompt_logprobs` (per prompt token; -1 = full vocab), capped by `max_logprobs`. — [vllm/sampling_params.py](https://github.com/vllm-project/vllm/blob/main/vllm/sampling_params.py). Quantization hardware matrix: AWQ Turing/Ampere/Ada/Hopper (not Volta); GPTQ Volta+; Marlin (GPTQ/AWQ/FP8/FP4) Ampere+ (Turing partial); bitsandbytes Volta+; GGUF supported (Volta+). — [vLLM quantization docs](https://github.com/vllm-project/vllm/blob/main/docs/features/quantization/README.md). `gpu_memory_utilization` default 0.9 (fraction of VRAM vLLM claims). — [Unsloth vLLM engine args](https://unsloth.ai/docs/basics/inference-and-deployment/vllm-guide/vllm-engine-arguments)
- **ExLlamaV3/TabbyAPI**: READMEs do not mention logprob output (grep found nothing). — [exllamav3 README](https://github.com/turboderp-org/exllamav3); [tabbyAPI README](https://github.com/theroyallab/tabbyAPI)
- **EvalPlus backends** include `hf`, `vllm`, `openai` (any OpenAI-compatible server, e.g. llama-server), `ollama`, `gptqmodel`, `anthropic`, `google`, `bedrock`, `hf_gaudi`. — [EvalPlus README](https://github.com/evalplus/evalplus)
- **Throughput**: llama.cpp's own table (hardware not stated in README) for Llama-3.1-8B: text generation @128 tokens — Q2_K 79.9, Q3_K_M 71.7, Q4_K_M 71.9, Q5_K_M 67.2, Q6_K 58.7, Q8_0 50.9 t/s; prompt processing ~750–865 t/s. — [llama.cpp quantize README](https://github.com/ggml-org/llama.cpp/blob/master/tools/quantize/README.md)
- Qwen2.5-Coder-7B Q4_K_M on RTX 4060 8 GB: 28–35 tok/s, ~5 GB VRAM (measured via Ollama verbose). — [bswen blog, Mar 2026](https://docs.bswen.com/blog/2026-03-27-rtx-4060-token-speed-benchmark-coding/)
- Llama-3.1-8B Q4_K_M on RTX 3060 Ti/3070/4060: 15–30 tok/s; llama.cpp ~3–10% faster than Ollama on NVIDIA. — [SitePoint 8 GB guide](https://www.sitepoint.com/optimizing-local-llms-low-end-hardware-8gb/)
- Another guide claims 40+ tok/s for 7–8B Q4_K_M on 8 GB cards (backend unspecified). — [localllm.in Ollama VRAM guide](https://localllm.in/blog/ollama-vram-requirements-for-local-llms)

### Inferences
- Recommended stack: **llama.cpp `llama-server`** (CUDA build) with `-ngl 99 -np 4..8 -c <slots×ctx> --temp 0` and `n_probs: 5–20` per request. It is the only stack here that natively runs every GGUF k-quant level (Q2_K…Q8_0) on one engine, which keeps the inference stack constant across bit-widths — important for attributing effects to quantization. Use vLLM (AWQ/GPTQ 4-bit) or HF transformers+bitsandbytes (`output_scores=True` / `compute_transition_scores` — not verified here) as a secondary arm to test "quant method" vs "bit-width".
- Avoid relying on Ollama's OpenAI-compatible endpoint for logprobs; use its native `/api/chat` if Ollama is used.
- Single-stream decode on 8 GB cards is memory-bandwidth-bound: lower quant = faster (Q8_0 ~30% slower than Q4_K_M in llama.cpp's table). Parallel slots increase aggregate throughput but share the small KV budget.

### Gaps
- No primary `llama-bench` numbers for RTX 3060 Ti / 3070 / 4060 found (community scoreboard on GitHub discussions not reachable); throughput figures above are from blogs.
- Aggregate (multi-slot) throughput on 8 GB cards not documented in sources found.
- ExLlamaV2/V3 logprob support unconfirmed; transformers logprob API not verified in this session.

## Q5. Evaluation harnesses, sandboxing, and compute estimates for ~10k–40k short generations

### Takeaway
EvalPlus (HumanEval+/MBPP+) is the natural primary harness — it supports OpenAI-compatible and Ollama backends and ships a Docker image for safe execution; bigcode-evaluation-harness disables execution by default and recommends generating then executing in Docker; LiveCodeBench uses vLLM and timeout-tuned local execution. A 10k–40k-generation study is feasible on one 8 GB GPU in roughly one to a few days of GPU time per model-quant sweep.

### Cited Findings
- **EvalPlus**: "Safe code execution within Docker" — generate with any backend, then `docker run --rm --pull=always -v $(pwd)/evalplus_results:/app ganler/evalplus:latest evalplus.evaluate --samples ...`; v0.1.6 added configurable, by-default-conservative timeouts and HumanEval+ ground-truth fixes; backends listed above. — [EvalPlus README](https://github.com/evalplus/evalplus)
- **bigcode-evaluation-harness**: `--allow_code_execution` is off by default (read the warning); use `--generation_only` then execute generations in its Docker image `ghcr.io/bigcode-project/evaluation-harness` (separate `-multiple` image for MultiPL-E); can run execution on a separate CPU machine/container for security. — [bigcode-evaluation-harness README](https://github.com/bigcode-project/bigcode-evaluation-harness)
- **LiveCodeBench**: releases v1 (400 problems, May 2023–Mar 2024) … v5 (880, to Jan 2025), v6 (1055 problems, May 2023–Apr 2025); `--release_version` flag; uses vLLM for open models; if results vary, lower `--num_process_evaluate` or raise `--timeout`. — [LiveCodeBench README](https://github.com/LiveCodeBench/LiveCodeBench)
- Energy/time cost scales with inference time; some low-bit methods (HQQ 3-bit) are dramatically *slower* (+1524% energy) than 4-bit. — [arXiv 2606.27205](https://arxiv.org/pdf/2606.27205)

### Inferences
- **Sandbox recipe** (best practice, not from a single source): generate on the host GPU; execute in a container with `--network none`, `--read-only` rootfs + tmpfs workdir, non-root user, `--memory 1g --cpus 1 --pids-limit 256`, per-test wall-clock timeout (EvalPlus defaults) and a global per-sample kill; never run generated code with network access (also relevant for package-hallucination studies — checking whether a hallucinated package exists should be done via registry metadata APIs, never `pip install`, to avoid slopsquatting payloads).
- **Benchmark sizes** (recalled, verify): HumanEval+ 164 problems; MBPP+ ~378 problems. With e.g. 10 perturbations × 542 problems ≈ 5.4k prompts per model-quant; × 6 quant levels ≈ 33k generations — in the 10k–40k range.
- **Compute estimate** (assume ~250 output tokens/generation, prompts ~300 tokens): 10k gens ≈ 2.5M output tokens; 40k ≈ 10M. At ~30 tok/s single-stream (7B Q4_K_M on RTX 4060-class) → 10k ≈ 23 h, 40k ≈ 93 h. With 4–8 llama-server parallel slots aggregate throughput plausibly 2–4× → 10k ≈ 6–12 h, 40k ≈ 1–2 days. 3B models ~2× faster, 1.5B ~3–4× faster; Q8_0 ~30% slower than Q4_K_M. Prompt processing (~800 t/s class) adds only ~10% overhead. Requesting top-N logprobs adds modest per-token overhead. Execution of EvalPlus tests is CPU-bound and comparatively cheap (minutes–hours), but must be budgeted for perturbed variants. These are back-of-envelope figures combining the throughput sources in Q4.
- Greedy decoding (temperature 0) + `n_probs` gives deterministic pass@1 plus token-level uncertainty features in one pass; sampling-based uncertainty (e.g., n=5–10 samples for semantic-consistency signals) multiplies cost by n — likely the dominant compute driver.

### Gaps
- No published wall-clock benchmarks of EvalPlus/LiveCodeBench runs on 8 GB consumer GPUs found.
- Exact current EvalPlus default timeouts and MBPP+ problem count not re-verified this session.
