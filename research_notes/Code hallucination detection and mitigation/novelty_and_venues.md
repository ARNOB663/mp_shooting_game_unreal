# Novelty and venues for "Perturb, Execute, Agree: Measuring, Detecting and Reducing Code Hallucinations in Local LLMs"

Research date: 2026-10-13. Method note: arxiv.org, dl.acm.org, aclanthology.org, openreview.net, conf.researchr.org and api.semanticscholar.org were all blocked from this environment (proxy CONNECT 403 / DNS failure), so every finding below comes from web-search result snippets and abstracts, not full texts. Treat numbers as abstract-level claims. Anything marked UNVERIFIED should be checked before it is cited.

## Q1. Closest 2024–2026 work on perturbation consistency / metamorphic prompt testing for code (Component 2: test-free detection by execution agreement across perturbed prompts)

### Takeaway
The core idea behind Component 2 has been published several times. Wang and Zhu's "Metamorphic Prompt Testing" (arXiv 2024) became an ICSE 2026 NIER paper titled "Automatic Validation of LLM-Generated Code with Prompt Paraphrasing". It paraphrases the prompt, generates code from each variant, and cross-validates execution behaviour. MRSQLGen (FSE 2026) and SQLHD (arXiv Dec 2025) do the same for SQL. Semantic Triangulation (OOPSLA 2026) reports that it beats the paraphrasing approach. What remains open is a systematic comparison: perturbation granularity (character, word, sentence), AUROC against sampling consistency, entropy and static checks, and results on small quantized local models.

### Cited Findings
- **Wang & Zhu, "Validating LLM-Generated Programs with Metamorphic Prompt Testing"** (arXiv 2406.06864, 2024). Core claim: correct code is internally consistent across versions and flawed code is not, so flaws show up when semantic relations across multiple generated versions are checked. On HumanEval it detected 75% of GPT-4's erroneous programs at an 8.6% false-positive rate. — [arXiv 2406.06864](https://arxiv.org/abs/2406.06864)
- **Wang & Zhu, "Automatic Validation of LLM-Generated Code with Prompt Paraphrasing"**, ICSE 2026 NIER (UT San Antonio; talk on 16 Apr 2026 in Rio). Pipeline: paraphrase the prompt, generate code per variant, then cross-validate. It detects 70.6% of GPT-4o's erroneous HumanEval programs at a 6.1% FPR. ACM DOI 10.1145/3786582.3786809, pp. 51–55 (a 5-page NIER paper). — [researchr ICSE 2026 NIER](https://conf.researchr.org/details/icse-2026/icse-2026-nier/11/Automatic-Validation-of-LLM-Generated-Code-with-Prompt-Paraphrasing); [ACM DL](https://dl.acm.org/doi/10.1145/3786582.3786809)
  - **This is the single closest prior work to Component 2.** It uses sentence-level paraphrase, frontier models (GPT-4/4o) and HumanEval. As far as the snippets show, it has no character/word perturbations, no AUROC comparison against entropy or sampling-consistency baselines, and no local or quantized models.
- **MRSQLGen, "Validating LLM-Generated SQL Queries through Metamorphic Prompting"** (Li Lin, Qinglin Zhu, Jintai Hong, Chong Wang, Yang Liu, Rongxin Wu; ESEC/FSE 2026 research track; PACMSE, DOI 10.1145/3797146). It rewrites the input prompt with task-specific transformation rules derived from a hallucination taxonomy and checks whether the outputs stay consistent, with no ground-truth SQL needed. Evaluated on Spider and Bird with 5 LLMs including GPT-4o, it outperforms state-of-the-art hallucination detectors in precision and recall. — [ACM DL 10.1145/3797146](https://dl.acm.org/doi/10.1145/3797146); [Li Lin researchr profile](https://conf.researchr.org/profile/conf/lilin); [citedrive record](https://www.citedrive.com/en/discovery/validating-llm-generated-sql-queries-through-metamorphic-prompting/)
- **SQLHD, "Hallucination Detection for LLM-based Text-to-SQL Generation via Two-Stage Metamorphic Testing"** (Bo Yang, Yinfen/Yifen Xia, Weisong Sun, Yang Liu; arXiv 2512.22250, 24 Dec 2025). It needs no ground truth. Stage 1 uses metamorphic relations to catch schema-linking errors and stage 2 catches logical errors, reaching F1 69.36–82.76%, ahead of LLM self-evaluation baselines. — [arXiv 2512.22250](https://arxiv.org/html/2512.22250v1)
- **Semantic Triangulation / just-tri-it** (Yihan Dai, Sijie Liang, Haotian Xu, Peichu Xie, Sergey Mechtaev; PACMPL OOPSLA2 2026; arXiv 2511.12288). It transforms the problem into a "dissociative" variant with a verifiable solution mapping and checks consistency across the two. On LiveCodeBench and CodeElo with GPT-4o and DeepSeek-V3, it reports +21% reliability over high-confidence selection at threshold 0.5. It also explicitly reports that it "significantly outperformed" Wang & Zhu's paraphrase-based (non-dissociative) metamorphic approach. — [arXiv 2511.12288](https://arxiv.org/abs/2511.12288); [GitHub msv-lab/just-tri-it](https://github.com/msv-lab/just-tri-it); [ACM DL 10.1145/3839525](https://dl.acm.org/doi/10.1145/3839525)
  - This is a direct threat to the framing. A reviewer will ask why semantics-preserving perturbation should beat triangulation, given that triangulation already beat paraphrase-MPT.
- **DrHall, "Detecting and Reducing the Factual Hallucinations of LLMs with Metamorphic Testing"** (Wu et al., FSE 2025). Black-box metamorphic relations for detecting and reducing factual hallucination, evaluated on 3 datasets covering natural-language and code data. — [FSE 2025 researchr](https://conf.researchr.org/details/fse-2025/fse-2025-research-papers/129/Detecting-and-Reducing-the-Factual-Hallucinations-of-Large-Language-Models-with-Metam)
- **MetaQA, "Hallucination Detection in LLMs with Metamorphic Relations"** (Yang et al., arXiv 2502.15844; listed as FSE 2025). Prompt mutations used for black-box hallucination detection, on general QA rather than code. — [arXiv 2502.15844](https://arxiv.org/pdf/2502.15844)
- **Chan et al. (2025)** test the stability of code LLMs with symmetric metamorphic relations (prompt case changes, duplication, paraphrasing) and check that semantics-preserving prompts yield functionally equivalent code. Found via the MT+LLM survey; the primary source was not located. — [MT–LLM survey arXiv 2605.13898](https://arxiv.org/html/2605.13898v1)
- **Survey: "Bidirectional Empowerment of Metamorphic Testing and Large Language Models: A Systematic Survey"** (arXiv 2605.13898, May 2026). Useful for collecting all MT-for-LLM-code work and as a related-work anchor. — [arXiv 2605.13898](https://arxiv.org/pdf/2605.13898)
- **Robustness under perturbation (correctness, not detection):** ReCode (ACL 2023) built semantics-preserving transformations of docstrings and code, with over 90% of perturbed prompts judged meaning-preserving by humans. — [ACL Anthology 2023.acl-long.773](https://aclanthology.org/2023.acl-long.773.pdf). Related: NLPerturbator (arXiv 2406.19783) — [arXiv](https://arxiv.org/pdf/2406.19783); multi-language robustness (arXiv 2504.19108) — [arXiv](https://arxiv.org/pdf/2504.19108); prompt stability over 14 models on HumanEval with emotion- and personality-driven variants (arXiv 2509.13680) — [arXiv](https://arxiv.org/html/2509.13680v1).
- **"Minimal Prompt Perturbations Lead to Code Vulnerabilities"** (Sternfeld, Kucharavy, Dolamic; arXiv 2605.29737, 28 May 2026). Single-character or token edits can flip secure code to vulnerable across 3 main models (CodeLlama-70B, DeepSeek-Coder-33B, Qwen3-Coder-30B) and 5 languages. Hidden-state probes predict input-handling vulnerabilities at mean AUC 0.753 and secure-defaults vulnerabilities at 0.674. The appendix includes Qwen2.5-Coder-3B. — [arXiv 2605.29737](https://arxiv.org/pdf/2605.29737)
  - This is the closest paper to the character/word-level perturbation idea taken from Saha et al. / PromptAttack, but it targets security, not hallucination detection.
- **Execution-agreement uncertainty without prompt perturbation** (these are the baselines Component 2 must beat or match):
  - Sharma & David (U. Bristol), "Assessing Correctness in LLM-Based Code Generation via Uncertainty Estimation" (arXiv 2502.11620): semantic entropy over symbolic-execution clusters, plus a simplified uniform-distribution entropy. They report correlation with correctness ("strong" in v2) and abstention at a very low FPR (the snippet says below 0.02%; UNVERIFIED). — [arXiv 2502.11620](https://arxiv.org/html/2502.11620v3)
  - Ensemble-based UQ for code correctness (arXiv 2603.27098): cross-model disagreement, CrossHair symbolic clustering, and selective generation with abstention. — [arXiv 2603.27098](https://arxiv.org/html/2603.27098)
  - Functional Entropy (Bouchard et al., arXiv 2605.28500, May 2026): 5 LLMs, 3 languages, over 1,700 problems. NLI clustering collapses code into a single cluster, while LLM-judged functional equivalence gets the top AUROC in 11 of 15 model-benchmark settings. Released in UQLM. — [arXiv 2605.28500](https://arxiv.org/pdf/2605.28500)
  - Code-MUE (Ren, Xue, Ma, Huang; arXiv 2607.12273, Jul 2026): execution-based semantic interaction graph with Von Neumann entropy, Spearman up to −0.98 with correctness over 8 LLMs. A secondary review notes that its agreement metric is under-specified. — [arXiv 2607.12273](https://arxiv.org/html/2607.12273v2)
  - Introspective Uncertainty Estimation for LLM-based code generation (arXiv 2609.13975, Sep 2026). Only the title was seen; contents UNVERIFIED. — [arXiv 2609.13975](https://arxiv.org/pdf/2609.13975)

### Inferences
- "Execution agreement across perturbed prompts as a hallucination signal" is not novel as a concept. Wang & Zhu hold priority (arXiv 2024, ICSE-NIER 2026), MRSQLGen and SQLHD hold it for SQL, and DrHall/MetaQA for general hallucination. The planned paper must cite all of these and position itself as a systematic empirical study plus an extension, not as a new idea.
- Defensible novelty margins for Component 2:
  1. Perturbation granularity (character, word, sentence, following PromptAttack/Saha), set against Wang & Zhu's paraphrase-only approach.
  2. Head-to-head AUROC against sampling-consistency (same prompt, temperature sampling), token entropy/log-prob, symbolic or execution semantic entropy (Sharma & David), functional entropy, and static checks, all on the same models and test suites. The surveyed papers each compare against a subset. None found compares prompt-perturbation agreement with same-prompt sampling agreement on equal compute budgets.
  3. Ablations: does perturbation add signal beyond sampling at equal sample count? This is the question a reviewer will ask first.
  4. Small or quantized local models. All the paraphrase-MPT and triangulation results are on GPT-4/4o/DeepSeek-V3.
- Semantic Triangulation's reported win over paraphrase-MPT is a risk. Include it as a baseline or explicitly discuss why it was excluded (e.g., it needs problem-specific transformations and cannot be fully automated for every task).

### Gaps
- Could not read full texts (arXiv and ACM blocked), so whether Wang & Zhu's NIER paper reports AUROC, uses open models, or varies perturbation type is UNVERIFIED.
- Chan et al. 2025 was not located as a primary source.
- Could not check whether MRSQLGen or SQLHD evaluated any small or local models.

## Q2. Closest work to Component 3 (mitigation: API verifier + perturbed-prompt ensemble with execution clustering + abstention)

### Takeaway
Each piece of Perturb–Cluster–Verify exists separately. Execution clustering with abstention is in Ravuri & Amarasinghe 2025, CodeRefuser 2026 and Sharma & David 2025. Execution-fingerprint voting is in Semantic Voting 2026 and Functional Majority Voting 2026. API/library verifiers are in MARIN (FSE 2025), DAG/CloudAPIBench, Hallucination Inspector, and the static-analysis study of 2026. No paper found combines a perturbed-prompt ensemble (rather than same-prompt sampling) with execution clustering, abstention and an API verifier, and evaluates it on quantized local models. The novelty is therefore in the combination and evaluation, not in a new mechanism.

### Cited Findings
- **Ravuri & Amarasinghe (MIT), "Eliminating Hallucination-Induced Errors in LLM Code Generation with Functional Clustering"** (arXiv 2506.11021, 2025; also an MIT MEng thesis). A black-box wrapper samples many programs, clusters them by identical I/O on a self-generated test suite, and uses the largest cluster's size as confidence, with one threshold trading coverage for reliability. On LiveCodeBench, the error rate of returned answers falls from about 65% to 2% (0% at a conservative threshold) while 15.6% of problems are still answered. — [arXiv 2506.11021](https://arxiv.org/pdf/2506.11021); [MIT DSpace](https://dspace.mit.edu/handle/1721.1/162941)
  - This is the closest to the "cluster + abstain" part of Component 3. The difference is that it samples from the same prompt and has no prompt perturbation and no API verifier.
- **Task Abstention for LLMs in Code Generation / CodeRefuser** (arXiv 2605.17029, 2026). Clusters samples by execution output, then abstains when consensus is low (cluster-ratio mode) or when cluster entropy is high (semantic-entropy mode). — [arXiv 2605.17029](https://arxiv.org/pdf/2605.17029)
- **Semantic Voting: Execution-Grounded Consensus for LLM Code Generation** (arXiv 2605.08680, 2026). Compares output-pattern majority voting, weighted voting, MBR-Exec and SemanticVote, which clusters by execution fingerprints on LLM-generated inputs. The best execution-based selector beats output-pattern voting by 19–52 percentage points, and input quality is the largest factor. — [arXiv 2605.08680](https://arxiv.org/pdf/2605.08680)
- **Functional Majority Voting** (ICLR 2026 Test-Time Updates workshop; arXiv 2604.15618). Picks a representative solution by runtime execution on K test inputs without ground-truth outputs. — [arXiv 2604.15618](https://arxiv.org/pdf/2604.15618)
- **ConSelf, "Self-Improving Code Generation via Semantic Entropy and Behavioral Consensus"** (arXiv 2603.29292; ACM DOI 10.1145/3794763.3794798). Uses code semantic entropy over execution clusters as a curriculum signal. — [arXiv 2603.29292](https://arxiv.org/html/2603.29292)
- **"When Uncertainty Isn't Enough: An Empirical Study of Self-Correction in Code Generation"** (arXiv 2608.14659). Trains hidden-state probes to predict semantic entropy, using symbolic-execution clusters as labels. — [arXiv 2608.14659](https://arxiv.org/pdf/2608.14659)
- **API/library hallucination verifiers and mitigations:**
  - MARIN (FSE 2025 Industry; arXiv 2505.05057): hierarchical dependency mining plus dependency-constrained decoding, reporting an average 67.52% drop in MiHN and 73.56% in MaHR versus RAG. — [arXiv 2505.05057](https://arxiv.org/html/2505.05057v1); [ACM DL](https://dl.acm.org/doi/10.1145/3696630.3728569)
  - Jain et al. (Amazon), "On Mitigating Code LLM Hallucinations with API Documentation" (arXiv 2407.09726): CloudAPIBench. Documentation-augmented generation helps rare APIs but hurts common ones, so retrieval is triggered selectively via index lookup or a confidence threshold. — [arXiv 2407.09726](https://arxiv.org/pdf/2407.09726); [Amazon Science](https://www.amazon.science/publications/on-mitigating-code-llm-hallucinations-with-api-documentation)
  - "An Empirical Analysis of Static Analysis Methods for Detection and Mitigation of Code Library Hallucinations" (arXiv 2604.07755, 2026). Directly relevant to the "static checks" baseline and the API verifier; contents UNVERIFIED (only the title was seen). — [arXiv 2604.07755](https://arxiv.org/pdf/2604.07755)
  - Hallucination Inspector (arXiv 2604.20202): an AST-symbol verifier for API migration, checked against a documentation knowledge base. — [awesomepapers summary](https://awesomepapers.io/ai-for-code/papers/2604.20202)
  - "Library Hallucinations in LLM-Generated Code: A Risk Analysis Grounded in Developer Queries" (arXiv 2509.22202): AST import parsing checked against PyPI. — [arXiv 2509.22202](https://arxiv.org/pdf/2509.22202)
  - "Mitigating Errors in LLM-Generated Web API Invocations via RAG and Constrained Decoding" (arXiv 2607.05936); contents UNVERIFIED. — [arXiv 2607.05936](https://arxiv.org/pdf/2607.05936)
- **Paraphrase ensembles outside code-execution settings:** an aggregator summary says that averaging next-token probabilities across paraphrased prompts gives small gains on code benchmarks, and that paraphrase plus majority vote can beat self-consistency for mid-tier LLMs (UNVERIFIED secondary summary). — [Pith citations page](https://pith.science/citations/2f35c49f-1659-4509-85dc-cb2be4583b79). A Building Trust @ ICLR 2025 workshop paper ensembles instruction variations with majority voting, but on classification. — [arXiv 2504.02733](https://arxiv.org/pdf/2504.02733)

### Inferences
- Mitigation novelty is incremental. The defensible claim is that a perturbed-prompt ensemble diversifies errors more than temperature sampling, so execution clustering over perturbed-prompt outputs gives better risk–coverage curves than same-prompt clustering (Ravuri & Amarasinghe; CodeRefuser) at equal sample budgets on small models. Adding a cheap API verifier then removes the remaining API/library hallucinations that execution clustering misses when the test inputs do not exercise those paths. This claim must be shown empirically (risk–coverage or AURC curves, equal-budget comparisons).
- Required baselines for Component 3: same-prompt functional clustering (Ravuri & Amarasinghe-style), CodeRefuser-style abstention, MBR-Exec/SemanticVote, and documentation RAG for APIs.

### Gaps
- No paper was found that explicitly combines perturbed-prompt ensembles with execution clustering for code. A search of arXiv listings with full-text access would be needed to rule this out.

## Q3. Code hallucination in small / quantized local models (Component 4)

### Takeaway
This is the least-crowded component. Only one paper studies hallucination under quantization for code, and it is narrow: package hallucinations in shell/Go install commands on Qwen at FP/8-bit/4-bit. The other quantization work for code measures functional correctness or quality, not hallucination detection or mitigation. A quantization sweep reporting both hallucination rates and detector AUROC on 8 GB GPUs appears to be open.

### Cited Findings
- **"Secure or Suspect? Investigating Package Hallucinations of Shell Command in Original and Quantized LLMs"** (Md Nazmul Haque, Elizabeth Lin, Lawrence Arkoh, Biruk Tadesse, Bowen Xu; NC State; arXiv 2512.08213, Dec 2025). Five Qwen sizes at full precision, 8-bit and 4-bit, on SO, MBPP and paraphrase datasets. Quantization raises the package hallucination rate, and the 4-bit models degrade most (smaller 4-bit models described as "nearly unusable"). Vulnerability presence among correct packages rises as precision drops. — [arXiv 2512.08213](https://arxiv.org/html/2512.08213v1); [alphaXiv](https://www.alphaxiv.org/abs/2512.08213)
- **"Quantize with Confidence? An Empirical Study of Quantization for Code Generation"** (arXiv 2607.14181, Jul 2026). Six 4-bit methods (GPTQ, AWQ, QuIP#, AQLM, BitsAndBytes, GGUF Q4_K_M) on Qwen2.5-Coder-7B and CodeLlama-7B. 4-bit largely preserves functional correctness, but the method matters: AQLM is at or above FP baseline, QuIP# degrades most on complex tasks. Quality effects are small. — [arXiv 2607.14181](https://arxiv.org/html/2607.14181v1)
- **"Smaller = Weaker? Benchmarking Robustness of Quantized LLMs in Code Generation"** (arXiv 2506.22776). Quantized models were more resilient in 51.59% of adversarial cases versus 42.86% for full precision (from a secondary Pith summary; UNVERIFIED). — [Pith 2506.22776](https://pith.science/paper/2506.22776)
- **"Precision or Peril: A PoC of Python Code Quality from Quantized LLMs"** (arXiv 2411.10656): inconsistent quality effects, with recurring maintainability issues. — [arXiv 2411.10656](https://arxiv.org/html/2411.10656v1)
- **"Evaluating Quantized LLMs for Code Generation on Low-Resource Language Benchmarks"** (arXiv 2410.14766, 2024): Lua on CPU-only laptops with GGUF. 4-bit 7B models beat smaller unquantized models, but overall scores are under 50%. — [arXiv 2410.14766](https://arxiv.org/html/2410.14766v1)
- **"Evaluating the Impact of Post-Training Quantization on LLMs for Code Generation"** (ACM DOI 10.1145/3794763.3794810); details UNVERIFIED. — [ACM DL](https://dl.acm.org/doi/10.1145/3794763.3794810)
- **Quantized small models for program repair:** "How Small Is Enough?" (arXiv 2508.16499) and "Smaller Models, Unexpected Costs: Trade-offs in LLM Quantization for APR" (arXiv 2606.27205). Seen only via titles. — [arXiv 2606.27205](https://arxiv.org/pdf/2606.27205)
- **Package hallucination in frontier models, 2026 cohort** (arXiv 2605.17062, 16 May 2026): rates of 4.62% (Claude Haiku 4.5) to 6.10% (GPT-5.4-mini) on Python/JS prompts. Useful as a contrast with local models. — [arXiv 2605.17062](https://arxiv.org/abs/2605.17062)

### Inferences
- The "local models on 8 GB" angle is a real differentiator against Q1/Q2 work, which is almost entirely GPT-4o, DeepSeek-V3 or 30–70B models. It is also practically motivated, since many developers run Ollama or llama.cpp locally.
- The literature conflicts: correctness is roughly preserved at 4-bit (2607.14181), while package hallucination rises (2512.08213). A quantization sweep that measures both correctness and hallucination type, and checks whether detector AUROC degrades with bit-width, would directly address this tension. That is a good hook.

### Gaps
- No paper was found that measures whether hallucination-detection signals (entropy, consistency, perturbation agreement) stay calibrated under quantization. This looks open, but was not verifiable against full texts.

## Q4. Component 1 (per-model Code Hallucination Score) and recent surveys and taxonomies with open problems

### Takeaway
Taxonomies and benchmarks are well established: CodeHalu (AAAI 2025), Collu-Bench, CodeMirage, HalluCode, Lee et al.'s 2025 taxonomy survey, Gao et al.'s 2025 SLR (accepted at TOSEM), and a Sep 2026 taxonomy separating hallucination from ordinary errors. A new score therefore needs to be explicitly an aggregate over auto-detectable categories mapped onto an existing taxonomy (e.g., CodeHalu's mapping/naming/resource/logic types), not a new taxonomy.

### Cited Findings
- **Gao et al., "A Systematic Literature Review of Code Hallucinations in LLMs: Characterization, Mitigation Methods, Challenges, and Future Directions for Reliable AI"** (Cuiyun Gao + 7 co-authors; arXiv 2511.00776, 2 Nov 2025; listed as an ACM TOSEM article). Reviews 60 papers. Causes: data noise, exposure bias, insufficient semantic grounding. Mitigations: knowledge-enhanced generation, constrained decoding, post-editing. — [arXiv 2511.00776](https://arxiv.org/abs/2511.00776)
- **Lee, Song & Nam, "Hallucination by Code Generation LLMs: Taxonomy, Benchmarks, Mitigation, and Challenges"** (arXiv 2504.20799, Apr 2025). Taxonomy with 5 primary categories, plus a review of benchmarks, mitigations and open challenges. — [Semantic Scholar page](https://www.semanticscholar.org/paper/Hallucination-by-Code-Generation-LLMs:-Taxonomy,-Lee-Song/ab90297f5a95b79d67620c32d2474c9a1dfef8c8)
- **"Refusing the Impossible: A Taxonomy and Benchmark for Code Hallucination in LLMs"** (arXiv 2609.03267, Sep 2026). Separates code hallucination from ordinary code error. Relevant because the planned score must define hallucination versus bug. — [arXiv 2609.03267](https://arxiv.org/html/2609.03267v1)
- **CodeHalu** (Tian et al., AAAI 2025; arXiv 2405.00253): execution-based verification with mapping, naming, resource and logic hallucination types. — [arXiv 2405.00253](https://arxiv.org/pdf/2405.00253)
- **Collu-Bench** (Jiang et al., 2024): 13,234 instances from 5 datasets and 11 LLMs (generation and repair). Predicting hallucinations reached only 22.03–33.15% accuracy. — [ResearchGate record](https://www.researchgate.net/publication/384929903_Collu-Bench_A_Benchmark_for_Predicting_Language_Model_Hallucinations_in_Code)
- **CodeMirage** (Agarwal et al.): 1,137 Python problems from HumanEval/MBPP with ChatGPT-3.5-inserted hallucinations. Sizes vary across sources. — summarized in [arXiv 2609.03267](https://arxiv.org/html/2609.03267v1)
- **"Beyond Functional Correctness: Exploring Hallucinations in LLM-Generated Code"** (Beihang; Liu et al., source of HalluCode). — [BUAA record](https://research.buaa.edu.cn/en/publications/beyond-functional-correctness-exploring-hallucinations-in-llm-gen/)
- **"Beyond Document Grounding: Span-Level Hallucination Detection over Code, Tool Output, and Documents"** (arXiv 2607.00895, 2026): span-level detection that includes code. — [arXiv 2607.00895](https://arxiv.org/html/2607.00895)
- **General hallucination surveys for 2025–2026 context:** "A Concise Review of Hallucinations in LLMs and their Mitigation" (arXiv 2512.02527) — [arXiv](https://arxiv.org/pdf/2512.02527); "Hallucinations in LLMs: A Lifecycle-Based Survey" (arXiv 2608.26168) — [arXiv](https://arxiv.org/pdf/2608.26168).

### Inferences
- The specific open-problem lists in the Gao SLR and the Lee survey could not be read (full text blocked). Based on abstracts, they cover detection without ground truth, code-specific semantic grounding, and evaluation benchmarks. The planned paper should quote their "future directions" verbatim once the PDFs are accessible, to show that it answers a stated open problem.
- Collu-Bench's low prediction accuracy (22–33%) is a useful motivating citation for "detection is hard".

### Gaps
- Verbatim open-challenge lists from 2511.00776 and 2504.20799 could not be retrieved.

## Q5. Has Saha et al. (2025, Discover Data 3:20) been cited or extended? Follow-ups by the authors?

### Takeaway
I found no citing paper and no follow-up by the authors. Web searches for the title, the DOI string, and "Saha et al. 2025" with PromptAttack/HHEM returned only the paper itself (Springer, ResearchGate). Semantic Scholar and Google Scholar citation lists could not be reached. Given a mid-2025 Discover Data paper, a citation count of zero or near zero is plausible but UNVERIFIED.

### Cited Findings
- The paper: Anirban Saha, Binay Gupta, Anirban Chatterjee, Kunal Banerjee (all Walmart Global Tech, Bangalore). Discover Data 2025, vol. 3, art. 20, published around 29 May 2025. It uses PromptAttack perturbations plus Vectara's hallucination detection model on 8 foundation models and finds GPT-4 most robust. The acknowledgements say the work is independent research not endorsed by Walmart. The authors claim to be "the first" to use a perturbation-based methodology to evaluate robustness to hallucination. — [Springer](https://link.springer.com/article/10.1007/s44248-025-00041-7); [ResearchGate](https://www.researchgate.net/publication/392213537_You_believe_your_LLM_is_not_delusional_Think_again_a_study_of_LLM_hallucination_on_foundation_models_under_perturbation)
- An exact-DOI search ("s44248-025-00041-7") returned only the Springer page. — [Springer](https://link.springer.com/article/10.1007/s44248-025-00041-7)
- A search for Banerjee/Chatterjee Walmart hallucination work in 2026 surfaced no paper by these authors. — (search results only; no source to cite)
- Related, non-citing parallel work using prompt perturbation for hallucination:
  - "Evaluating the Effects of Prompt Perturbation on Bias and Hallucination in LLMs" (Yousefi et al., Springer LNCS chapter, 2025). — [Springer chapter](https://link.springer.com/chapter/10.1007/978-981-96-6588-4_25)
  - "Probing LLM Hallucination from Within: Perturbation-Driven Approach via Internal Knowledge" (arXiv 2411.09689). — [arXiv](https://arxiv.org/pdf/2411.09689)
  - "Flip-Flop Consistency: Unsupervised Training for Robustness to Prompt Perturbations" (arXiv 2510.14242). — [arXiv](https://arxiv.org/pdf/2510.14242)
  - "Shaking to Reveal" (arXiv 2506.02696): perturbation of intermediate representations for hallucination self-assessment. — [arXiv 2506.02696](https://arxiv.org/abs/2506.02696v1)
- HHEM context: Vectara's current open model is HHEM-2.1, which outputs a 0–1 factual consistency score. Which HHEM version Saha et al. used is UNVERIFIED. — [Vectara HHEM-2.1 blog](https://www.vectara.com/blog/hhem-2-1-a-better-hallucination-detection-model); [Vectara leaderboard](https://huggingface.co/spaces/vectara/leaderboard)

### Inferences
- Saha et al.'s "first to use perturbation" claim is contestable. Wang & Zhu (2024) and the metamorphic-testing line predate it for code. The planned paper should therefore frame Saha et al. as the inspiration for the perturbation scheme and the ground-truth-free consistency idea, not as the sole antecedent. An extension that relies only on Saha et al. would look under-researched to SE reviewers.
- HHEM is an NLI/summarization-consistency model. Functional Entropy (arXiv 2605.28500) reports that NLI-style clustering collapses code samples into a single cluster. That gives the planned paper a principled argument for replacing HHEM with execution agreement for code, which is a clean "what changes when you move from text to code" contribution.

### Gaps
- Citation count is unknown (Semantic Scholar, Google Scholar and ResearchGate were unreachable). Check Google Scholar's "Cited by" manually.
- Whether the authors have unpublished or arXiv follow-ups could not be confirmed.

## Q6. Frank novelty assessment and strongest framing

### Takeaway
Novelty is moderate to low on the mechanism and moderate on the empirical study. Execution agreement under prompt perturbation (Wang & Zhu ICSE-NIER 2026; MRSQLGen FSE 2026; SQLHD 2025), execution clustering with abstention (Ravuri & Amarasinghe 2025; CodeRefuser 2026), and API verifiers (MARIN, DAG) all exist. The strongest framing is a rigorous, reproducible empirical study on consumer hardware. It would answer: (i) does prompt perturbation add detection signal beyond same-prompt sampling at equal budget, (ii) which perturbation granularity works, (iii) how quantization shifts hallucination rates and detector calibration, and (iv) what a cheap, test-free mitigation achieves on 8 GB GPUs.

### Cited Findings
- Paraphrase-based execution agreement already exists (70.6% detection at 6.1% FPR, GPT-4o). — [ICSE 2026 NIER](https://conf.researchr.org/details/icse-2026/icse-2026-nier/11/Automatic-Validation-of-LLM-Generated-Code-with-Prompt-Paraphrasing)
- Triangulation beats paraphrasing. — [arXiv 2511.12288](https://arxiv.org/abs/2511.12288)
- Functional clustering with abstention already exists (65% to 2% error at 15.6% coverage). — [arXiv 2506.11021](https://arxiv.org/pdf/2506.11021)
- The quantization–hallucination link is studied only for package hallucinations. — [arXiv 2512.08213](https://arxiv.org/html/2512.08213v1)
- Execution-based UQ is crowded in 2026 (Functional Entropy, Code-MUE, ensemble UQ). — [arXiv 2605.28500](https://arxiv.org/pdf/2605.28500); [arXiv 2607.12273](https://arxiv.org/html/2607.12273v2); [arXiv 2603.27098](https://arxiv.org/html/2603.27098)

### Inferences
- **Recommended framing:** "Does perturbation help? A controlled study of test-free code-hallucination detection and mitigation for quantized local LLMs." Lead with the research questions, not with the method name. Possible RQs:
  1. How do hallucination types and rates vary across ≤8B models and quantization levels (Q8, Q6_K, Q5_K_M, Q4_K_M, Q3_K, etc.)?
  2. Does execution agreement across character/word/sentence perturbations beat same-prompt sampling agreement, token entropy, symbolic/functional entropy and static checks (AUROC/AUPRC) at matched sample budgets?
  3. Does detector calibration degrade with quantization?
  4. What risk–coverage trade-off does Perturb–Cluster–Verify reach compared with same-prompt functional clustering, and what does each component contribute (ablation)?
- **Negative results are publishable.** If perturbation adds nothing beyond sampling, that is a clean finding that directly qualifies Wang & Zhu and Saha et al. It would fit "Insights from Negative Results" or an MSR/EMSE-style empirical paper.
- **Reframe Component 1** as an aggregate of existing taxonomies (CodeHalu types, plus API/package hallucination via PyPI or import checks) instead of introducing a new taxonomy.
- **Must-cite list:** Wang & Zhu 2024/2026; MRSQLGen; SQLHD; DrHall; MetaQA; Semantic Triangulation; Ravuri & Amarasinghe; Sharma & David; Functional Entropy; Code-MUE; CodeRefuser; Semantic Voting; MARIN; Jain et al. DAG; 2512.08213; 2607.14181; Gao SLR; Lee survey; CodeHalu; Collu-Bench; ReCode; 2605.29737.
- **Risk:** the arXiv field is moving fast (several relevant papers from May–Sep 2026). Re-run an arXiv search right before submission.

### Gaps
- Without full-text access, I cannot rule out that one 2026 paper already compares perturbation agreement with sampling agreement on small models. Do an arXiv full-text search for "paraphrase" + "semantic entropy" + "code" before submission.

## Q7. Venues, deadlines (as of 2026-10-13), and suitability for a student or independent first author

### Takeaway
The nearest realistic targets are ICSE 2027 NIER (23 Oct 2026; the same track that accepted Wang & Zhu's paraphrase paper), MSR 2027 technical papers (around 21–23 Oct 2026, tight), ICSE 2027 SRC (13 Nov 2026, student-only), LLM4Code 2027 @ ICSE (tentatively 13 Nov 2026), ARR January 2027 for ACL 2027 (4 Jan 2027), and ACL 2027 workshops (direct submission around 25 Apr 2027). Journals (EMSE, TOSEM, IST, JSS) have rolling submission. NAACL 2027's ARR deadline (12 Oct 2026) and FSE 2027's (2–3 Oct 2026) have already passed.

### Cited Findings
**Software engineering venues**
- **ICSE 2027** (Dublin, 25 Apr – 1 May 2027):
  - NIER submission deadline Fri 23 Oct 2026 AoE. HotCRP shows Sat 24 Oct 2026, 12 AM EDT. Notification 18 Dec 2026, camera-ready 20 Jan 2027. — [ICSE 2027 NIER](https://conf.researchr.org/track/icse-2027/icse-2027-new-ideas-and-emerging-results--nier-); [HotCRP](https://icse2027-nier.hotcrp.com/)
  - ACM Student Research Competition: submissions due Fri 13 Nov 2026, notification 18 Dec 2026. — [ICSE 2027 SRC](https://conf.researchr.org/track/icse-2027/icse-2027-src)
  - Journal-first track: submissions due 30 Oct 2026, for papers already accepted in journals. — [ICSE 2027 Journal-first](https://conf.researchr.org/track/icse-2027/icse-2027-journal-first-papers)
  - Research track: single cycle; notification listed as 20 Oct 2026, so the deadline has passed. Aggregators conflict (one lists a tentative Cycle 2 on 8 Feb 2027; UNVERIFIED and probably wrong given the single-cycle statement). — [ICSE 2027 research track](https://conf.researchr.org/track/icse-2027/icse-2027-research-track); [Bibby](https://trybibby.com/conference-deadlines/icse-2027)
  - Page-length precedent: Wang & Zhu's ICSE-NIER 2026 paper occupies pp. 51–55, i.e. 5 pages (likely 4 + 1 references; exact limit UNVERIFIED). — [ACM DL](https://dl.acm.org/doi/10.1145/3786582.3786809)
- **LLM4Code 2027** (workshop @ ICSE 2027, Dublin): deadline 13 Nov 2026, workshop 1 May 2027, both listed as tentative by an aggregator. — [se-deadlines.github.io](https://se-deadlines.github.io/)
- **MSR 2027** (Dublin, 26–27 Apr 2027, co-located with ICSE):
  - Technical papers: official HotCRP shows 21 Oct 2026 (07:59 EDT). An aggregator lists the abstract on 20 Oct and the full paper on 23 Oct 2026; sources conflict. — [MSR 2027 dates](https://2027.msrconf.org/dates); [MSR 2027 HotCRP](https://msr2027.hotcrp.com/)
  - Mining Challenge papers: abstract 18 Dec 2026, paper 23 Dec 2026, camera-ready 26 Jan 2027. Only relevant if the paper uses the announced challenge dataset. — [MSR 2027 Mining Challenge](https://2027.msrconf.org/track/msr-2027-mining-challenge)
  - Industry track: 18 Dec 2026. — [MSR 2027 Industry](https://2027.msrconf.org/track/msr-2027-industry-track)
  - Registered Reports track with EMSE (two-stage: pre-register the study, then publish in EMSE). A good fit for a planned empirical study whose experiments are not yet run. Dates UNVERIFIED. — [MSR 2027 Registered Reports](https://2027.msrconf.org/track/msr-2027-registered-reports)
- **FSE 2027** (Shenzhen, 12–16 Jul 2027): deadline 2 Oct 2026, extended to 3 Oct AoE after a HotCRP outage. **Passed.** The next FSE (2028) cycle would likely be around Sep–Oct 2027 (inference). — [FSE 2027 research papers](https://conf.researchr.org/track/fse-2027/fse-2027-papers); [PaperPilot](https://www.getpaperpilot.com/deadlines/fse-2027.html)
- **ASE 2027:** no dates found. ASE deadlines have historically been in late spring (inference, UNVERIFIED).
- **ACM TRUST 2027** (7–9 Mar 2027): research papers due 31 Oct 2026. A trust-focused ACM venue, not ACL. — [ACM TRUST 2027 CFP](https://eigtrust.acm.org/trust2027/cfp/)
- A consolidated SE deadline tracker: [se-deadlines.github.io](https://se-deadlines.github.io/)

**NLP venues (ACL Rolling Review)**
- **NAACL 2027** (San Francisco, 1–5 Jun 2027): ARR submission 12 Oct 2026, **passed yesterday**. Commitment 23 Dec 2026, notification 10 Feb 2027. — [NAACL 2027 CFP](https://www.aclweb.org/portal/content/call-main-conference-papers-naacl-2027)
- **ACL 2027** (Kyoto, 17–22 Aug 2027): ARR submission 4 Jan 2027. Commitment and notification TBD. — [ACL 2027 main call](https://2027.aclweb.org/calls/main/); [ACL 2027 first CFP](https://www.aclweb.org/portal/content/acl-2027-first-call-papers)
- **EACL 2027** (Athens, 9–14 Mar 2027): used the Aug 2026 ARR cycle (3 Aug 2026), so it has passed. — [ELRA Corpora list](https://list.elra.info/mailman3/hyperkitty/list/corpora@list.elra.info/thread/P2VBDJTDVUGDBRDTOOMYK4KW6NLDXTYC/)
- ARR moved from 8-week to 10-week cycles in May 2025, and authors must register as reviewers. — [ARR dates.md](https://github.com/acl-org/aclrollingreview/blob/main/dates.md)
- **ACL 2027 workshops:** joint workshop proposals closed 4 Sep 2026, with a direct paper-submission deadline for accepted workshops listed as 25 Apr 2027. Which workshops (TrustNLP, Insights from Negative Results, etc.) were accepted is not yet confirmed. — [Joint Call for Workshop Proposals 2027](https://www.aclweb.org/portal/content/joint-call-workshops-proposals-2027)
- **TrustNLP:** its 6th edition was at ACL 2026, with papers due 5 Mar 2026. The series has run at NAACL/ACL since 2021, so a 2027 edition is likely but UNCONFIRMED. — [TrustNLP @ ACL 2026](https://www.aclweb.org/portal/content/6th-trustworthy-nlp-workshop-acl-2026); [TrustNLP site](https://trustnlpworkshop.github.io/); [ACL Anthology TrustNLP venue](https://aclanthology.org/venues/trustnlp/)

**Journals (rolling)**
- **ACM TOSEM** published the Gao et al. code-hallucination SLR, so topic fit is confirmed. — [arXiv 2511.00776](https://arxiv.org/abs/2511.00776)
- **Empirical Software Engineering (Springer):** rolling submission, plus the MSR Registered Reports route. — [MSR 2027 RR](https://2027.msrconf.org/track/msr-2027-registered-reports)
- Discover Data / Discover AI (Springer, open access): the Saha et al. paper appeared in Discover Data. APC amount not found.

### Inferences
- **Best fit for a student or independent first author, by realism:**
  1. **ICSE 2027 NIER** (23 Oct 2026): exact topical precedent (Wang & Zhu 2026). A short paper on "perturbation granularity + local quantized models: preliminary results" is plausible but only 10 days away. Feasible only if pilot results already exist.
  2. **LLM4Code 2027 @ ICSE** (around 13 Nov 2026, tentative): a workshop with friendlier reviewing, directly on-topic. This is the most realistic near-term target for a first paper.
  3. **ICSE 2027 SRC** (13 Nov 2026): only for enrolled students. It is an extended abstract (typically 2 pages; UNVERIFIED) and gives mentorship and visibility.
  4. **ARR January 2027 → ACL 2027** (4 Jan 2027): suits the full study with a hallucination/consistency framing (TrustNLP-style). Main-track ARR long papers are 8 pages and short papers 4 (standard ARR limits; not re-verified here). Findings is a common outcome.
  5. **ACL 2027 workshops** (around Apr 2027 direct submission): TrustNLP, Insights from Negative Results (if perturbation turns out not to help), Student Research Workshop. Workshop acceptances are pending.
  6. **MSR 2027 Registered Report → EMSE:** good for pre-registering the quantization sweep. It reduces the "results not good enough" risk, which suits independent researchers.
  7. **Journals (EMSE, IST, JSS, TOSEM):** for the full four-component version with all baselines. TOSEM is the most competitive. IST and JSS are realistic for a thorough empirical study.
- **Probably skip:** MSR 2027 technical papers (8 days away, and the topic is more empirical-LLM than repository-mining), FSE 2027 (passed), NAACL 2027 (passed).
- **Independent-author note:** ARR's reviewer-registration requirement means an independent author must serve as a reviewer (the ARR dates.md says not all authors receive assignments, but non-compliance risks desk rejection). SE venues (ICSE, MSR) and their workshops have no such requirement as far as found.

### Gaps
- Exact page limits for ICSE 2027 NIER, SRC, LLM4Code 2027, and the MSR 2027 technical track were not retrieved (researchr was blocked).
- Not found: ASE 2027, DL4C 2027 (the ICLR workshop, if it runs; ICLR 2027 workshop deadlines are usually around Feb 2027, UNVERIFIED), Insights from Negative Results 2027, ACL 2027 SRW dates, and the Discover AI/Data APC.
- The LLM4Code 2027 date comes from an aggregator and is marked tentative.
