# Test-free detection of hallucinated / incorrect LLM-generated code (perturbation-consistency and uncertainty methods)

> Method note for the report writer: arxiv.org, dl.acm.org, openreview.net, aclanthology.org and GitHub repo contents could NOT be fetched directly from this environment (DNS or proxy refusal). Every number below comes from web-search result snippets of the primary pages (the URLs given), not from reading the full PDFs. Numbers marked **[verify]** came out differently across versions, or only from secondary snippets. Check them against the PDF before they go into the paper.

## Q1. Prompt-perturbation robustness of code LLMs (ReCode and follow-ups)

### Takeaway
ReCode (ACL 2023) set up the standard: 30+ semantics-preserving transformations of docstrings, function and variable names, code syntax and code format, scored with worst-case metrics (Robust Pass_s@k, Robust Drop, Robust Relative). Models were most sensitive to syntax perturbations. NLPerturbator (2024, TOSEM) narrowed the focus to natural-language prompt variations (18 categories) and found average relative Pass@1 drops of about 4.8 to 6.1% and a maximum of 21.2%. All of these papers measure robustness against ground-truth tests. None uses the perturbation response as a correctness detector.

### Cited Findings
- ReCode: Wang et al., "ReCode: Robustness Evaluation of Code Generation Models", ACL 2023 (arXiv 2212.10264). It is built on HumanEval and MBPP and applies "over 30 natural transformations" grouped into docstrings, function/variable names, code syntax and code format — [ACL PDF](https://aclanthology.org/2023.acl-long.773.pdf); [arXiv](https://arxiv.org/abs/2212.10264); code at [amazon-science/recode](https://github.com/amazon-science/recode)
- ReCode metrics:
  - Robust Pass_s@k (RP_s@k) is a worst-case measure. A problem counts as passed only if the model is correct on all s perturbed variants of its prompt, with correctness checked by unit tests.
  - Robust Drop_s@k is the relative decline from nominal Pass@k.
  - Robust Relative_s@k also counts flips where the model fails the original prompt but passes a perturbed one.
  - Source: [arXiv html](https://arxiv.org/html/2212.10264v1)
- ReCode headline findings, quoted from the abstract:
  - "(1) diverse pretraining corpus and larger model size can help improve the model worst-case robustness, but models may learn to generalize in a non-robust way"
  - "(2) code generation models are most sensitive to syntax perturbations". Syntax perturbations "often result in the most performance drops".
  - MBPP was harder than HumanEval overall.
  - Source: [arXiv](https://arxiv.org/abs/2212.10264)
- ReCode example numbers **[verify; secondary review snippet]**:
  - HumanEval RP5@1: CodeGen-16B-mono 0.195, InCoder-6B 0.067, GPT-J-6B 0.037.
  - Docstring RD5@1: CodeGen-16B-mono 50.00%, InCoder-6B 56.00%, GPT-J-6B 10.98%. The low GPT-J figure reflects its low nominal pass rate.
  - Source: [Liner review](https://liner.com/review/recode-robustness-evaluation-code-generation-models)
- CodeFort ("Robust Training for Code Generation Models", 2024) reuses ReCode with s=10 for robust training and evaluation — [arXiv 2405.01567](https://arxiv.org/html/2405.01567v1)
- NLPerturbator: Chen et al., "Studying the Robustness of Code LLMs to Natural Language Variations", ACM TOSEM (arXiv 2406.19783, 2024).
  - It covers 18 categories of NL perturbations plus 3 combinations.
  - The declarative-to-interrogative perturbation lowers average Pass@1 to 30.3% on HumanEval (4.8% relative decrease) and 44.2% on MBPP (6.1% relative decrease).
  - The maximum decrease is 21.2%, as summarised by CREME.
  - The arXiv version says 6 code LLMs were tested; the ACM abstract says 7 **[verify]**.
  - Sources: [arXiv PDF](https://arxiv.org/pdf/2406.19783); [ACM](https://dl.acm.org/doi/10.1145/3745764); [CREME summary](https://arxiv.org/html/2507.16407)
- CREME (arXiv 2507.16407, 2025) uses layer-aware model editing on NLPerturbator-perturbed HumanEval/MBPP and reports "63% average improvement in Pass@1 over the base LLM on perturbed prompts" — [arXiv](https://arxiv.org/html/2507.16407)
- Multilingual robustness follow-up: "A Multi-Language Perspective on the Robustness of LLM Code Generation" (arXiv 2504.19108, 2025) — [arXiv](https://arxiv.org/html/2504.19108v1)
- A related caveat from a vulnerability-detection study: paraphrased prompts produce much more output variability than repeated identical prompts, even under deterministic settings — [Springer chapter, "Prompting Instability"](https://link.springer.com/chapter/10.1007/978-981-95-4969-6_18) (taken from the search snippet; the page itself is unreachable here)

### Inferences
- Robustness work shows that prompt perturbations change what code models produce, so the perturbation signal is real. But ReCode and NLPerturbator report aggregate pass-rate drops, which require tests. Neither turns per-problem instability into a test-free correctness score. That step is the gap a perturbation-consistency detector would fill.
- ReCode's syntax and format perturbations act on the code prompt (partial code). Docstring and NL perturbations (character/word/sentence) map most directly onto Saha et al.'s character/word/sentence scheme.

### Gaps
- Full ReCode per-perturbation tables were not retrieved, and the exact RP/RD numbers come from a secondary review.
- No study was found that reports per-problem correlation between perturbation instability and test failure in ReCode or NLPerturbator data.

## Q2. Consistency and semantic-entropy detection, adapted to code; consistency across perturbed prompts

### Takeaway
Semantic entropy (Kuhn et al. ICLR 2023; Farquhar et al. Nature 2024, average AUROC 0.790 on QA) has been adapted to code by replacing NLI clustering with execution or symbolic equivalence. Examples:
- symbolic clustering (Sharma & David 2025)
- incoherence / DiffTrust (Valentin et al. 2025, about 2/3 of wrong programs caught with no reported false positives)
- functional clustering (Ravuri & Amarasinghe 2025, error rate of returned answers 65% to 2%)
- Code-MUE (2026)

Almost all of these use repeated sampling from the same prompt. The only works that measure consistency across perturbed prompts are Wang & Zhu's metamorphic / paraphrase prompt testing (arXiv 2024; ICSE 2026 NIER) and, with non-equivalent problem transformations, Semantic Triangulation (OOPSLA 2026).

### Cited Findings
**General-text foundations**
- Farquhar et al., "Detecting hallucinations in large language models using semantic entropy", Nature 630 (2024). "Averaged across the 30 combinations of tasks and models we study, semantic entropy achieves the best AUROC value of 0.790." AUROC is read as the probability that a random correct answer gets higher confidence than a random incorrect one — [Nature](https://www.nature.com/articles/s41586-024-07421-0)
- Kuhn et al., "Semantic Uncertainty", ICLR 2023 (arXiv 2302.09664) introduced meaning-clustering entropy. Its numbers were not retrieved — [cited via search](https://www.nature.com/articles/s41586-024-07421-0)
- Semantic Entropy Probes (Kossen et al. 2024, arXiv 2406.15927) predict semantic entropy from hidden states, avoiding multi-sampling — [arXiv](https://arxiv.org/pdf/2406.15927)
- SPUQ (Gao et al., EACL 2024) is perturbation-based uncertainty quantification for general LLMs. It argues that input perturbation such as paraphrasing "reveals significant prediction instability" and helps detect epistemic uncertainty. This is the general-text precedent for perturbation-based UQ — [ACL Anthology](https://aclanthology.org/2024.eacl-long.143.pdf)
- Saha (2025), "You believe your LLM is not delusional? Think again! a study of LLM hallucination on foundation models under perturbation", Discover Data 3, DOI 10.1007/s44248-025-00041-7.
  - Method: character-, word- and sentence-level query perturbation, then a consistency score between responses to the original and perturbed query.
  - Finding: GPT-4 hallucinated least.
  - Search results did not confirm that Vectara HHEM was the scorer **[verify]**.
  - Source: [ResearchGate listing](https://www.researchgate.net/publication/392213537_You_believe_your_LLM_is_not_delusional_Think_again_a_study_of_LLM_hallucination_on_foundation_models_under_perturbation)

**Semantic-entropy / consistency methods for code (same-prompt sampling)**
- Sharma & David, "Assessing Correctness in LLM-Based Code Generation via Uncertainty Estimation" (arXiv 2502.11620, 2025).
  - Programs are clustered by symbolic execution ("symbolic clustering" of symbolic output expressions).
  - Adding this clustering to NLG uncertainty methods restores their ability to predict correctness.
  - A uniform-distribution variant performs about as well as using LLM token probabilities.
  - An abstention policy "reduces incorrect outputs to near zero".
  - Source: [arXiv](https://arxiv.org/abs/2502.11620); [html v3](https://arxiv.org/html/2502.11620v3)
  - A follow-up reports its LiveCodeBench Pearson r of about -0.59 with partial pass@1 for GPT-3.5-Turbo-0125 **[verify]** — [arXiv 2605.09023](https://arxiv.org/html/2605.09023)
- Valentin, Madadi, Sapia, Böhme, "Estimating Correctness Without Oracles in LLM-Based Code Generation", retitled "Incoherence as Oracle-less Measure of Error in LLM-Based Code Generation" (arXiv 2507.00057, 2025).
  - Incoherence is estimated without an oracle and gives a lower bound on the error probability.
  - It "can automatically identify about two-thirds of incorrect programs without reports of false positives".
  - LLM rankings by incoherence agree strongly with pass@1 rankings.
  - Threats to validity: Python only.
  - Source: [arXiv](https://arxiv.org/abs/2507.00057); code at [DiffTrustRefiner](https://github.com/hfreud2s/DiffTrustRefiner)
- Ravuri & Amarasinghe (MIT), "Eliminating Hallucination-Induced Errors in LLM Code Generation with Functional Clustering" (arXiv 2506.11021, 2025).
  - Samples many programs, runs them on a self-generated test suite, and clusters programs with identical I/O. The share of the largest cluster is the confidence.
  - On LiveCodeBench, the error rate of returned answers falls from about 65% to 2%, and reaches 0% at a conservative threshold while answering 15.6% of prompts.
  - Black-box: no logits and no fine-tuning.
  - Source: [arXiv](https://arxiv.org/abs/2506.11021); code at [functional-clustering](https://github.com/20ChaituR/functional-clustering)
- Code-MUE (arXiv 2607.12273, 2026).
  - Builds a graph from execution agreement on probe inputs. The score is the Von Neumann entropy normalised by ln N.
  - Across 8 LLMs it reports Spearman correlation with correctness "up to -0.98" (best case) and beats lexical and embedding baselines.
  - It argues that NLP semantic entropy mistakes harmless token variation for real errors.
  - Source: [arXiv](https://arxiv.org/abs/2607.12273)
- ExeCRE (arXiv 2608.04439, 2026) scores each candidate's reliability by how stable its execution agreement is "over large and diverse constructed inputs", and uses it for self-correction — [arXiv](https://arxiv.org/html/2608.04439v1)
- Ensemble Semantic Entropy (arXiv 2603.27098, 2026) clusters programs from several models. It correlates better than single-model semantic entropy on LiveCodeBench and improves accuracy by 53.4% under strict false-positive-rate constraints — [arXiv](https://arxiv.org/html/2603.27098)
- "Self-Improving Code Generation via Semantic Entropy and Behavioral Consensus" (arXiv 2603.29292, 2026) reports that naive (embedding/NLI) semantic entropy performs only moderately on code, because text embeddings miss the functional effect of small symbolic changes — [arXiv](https://arxiv.org/pdf/2603.29292)
- Other symbolic and semantic-distance variants:
  - Semantic-distance entropy SDE/DSDE (arXiv 2605.09023) — [arXiv](https://arxiv.org/html/2605.09023)
  - "Inference-Time Code Selection via Symbolic Equivalence Partitioning" (arXiv 2604.06485) — [arXiv](https://arxiv.org/html/2604.06485v2)

**Consistency across perturbed or transformed prompts (closest to the planned paper)**
- Wang & Zhu (UTSA), "Validating LLM-Generated Programs with Metamorphic Prompt Testing" (arXiv 2406.06864, June 2024).
  - Premise: "intrinsic consistency always exists among correct code pieces but may not exist among flawed code pieces".
  - The method "varies the prompts to create paraphrase prompts, and then cross validate LLM-generated programs of multiple prompts".
  - On HumanEval it detects 75% of erroneous GPT-4 programs at an 8.6% false-positive rate.
  - Similarity among paraphrases causes both false positives and false negatives: near-identical paraphrases repeat the same wrong program and outvote correct ones.
  - Source: [arXiv](https://arxiv.org/abs/2406.06864); [html](https://arxiv.org/html/2406.06864v1)
- Its ICSE 2026 NIER version, "Automatic Validation of LLM-Generated Code with Prompt Paraphrasing", uses 5 paraphrases per prompt. With GPT-4o on HumanEval it detects 70.6% of erroneous programs at a 6.1% false-positive rate.
  - A search summary says the 70.6% comes from 17 erroneous programs (12/17 = 70.6%) **[verify]**.
  - Source: [ACM DOI](https://dl.acm.org/doi/10.1145/3786582.3786809); [ICSE 2026 NIER page](https://conf.researchr.org/details/icse-2026/icse-2026-nier/11/Automatic-Validation-of-LLM-Generated-Code-with-Prompt-Paraphrasing)
- Semantic Triangulation: "Reducing Hallucinations in LLM-Generated Code via Semantic Triangulation" (arXiv 2511.12288; OOPSLA 2026).
  - Each problem is rewritten into a "dissociative" variant (partial inversion, answer enumeration, problem decomposition) that likely needs a different algorithm. Solutions to the two problems are then checked for agreement under a hyperproperty, which decorrelates errors.
  - arXiv v1: 21% higher reliability than a probability-threshold-0.5 baseline on LiveCodeBench/CodeElo with GPT-4o and DeepSeek-V3. It can find correct solutions sampled with probability as low as 0.14.
  - ACM version: +16% probability of selecting a correct program over baselines (test generation, metamorphic testing, auto-formalised specs), with +7% reliability and +7% F1 **[verify which version]**.
  - Only 2% of cases selected an unreliable program because of a bad transformation.
  - Source: [arXiv](https://arxiv.org/html/2511.12288); [ACM](https://dl.acm.org/doi/10.1145/3839525); code at [just-tri-it](https://github.com/msv-lab/just-tri-it)

### Inferences
- Execution-based clustering clearly beats NLI or embedding clustering for code. The field has converged on behavioural equivalence on shared inputs, which is the same primitive the planned paper uses.
- Wang & Zhu is the main prior art for paraphrase-plus-execution cross-validation. It is evaluated mainly as a binary detector (detection rate and FPR) on HumanEval with one or two models and a small number of erroneous programs. It does not study character- or word-level noise, and it does not compare against semantic entropy, token log-probability or probe baselines by AUROC.
- Their reported failure mode (near-duplicate paraphrases sharing the same bug) suggests the planned paper should report how well perturbation diversity decorrelates errors, the issue Semantic Triangulation targets with deliberately non-equivalent transformations.

### Gaps
- No full-text check of how Wang & Zhu generate test inputs or aggregate votes. The search snippets did not say.
- Original SelfCheckGPT (Manakul et al., EMNLP 2023, arXiv 2303.08896) AUC-PR numbers and Kuhn et al. numbers were not retrieved.
- No AUROC for incoherence, functional clustering or metamorphic prompt testing was found. These papers report detection rate/FPR, error rate at abstention, or correlation, so direct comparison needs re-implementation.

## Q3. Token-level uncertainty and calibration for code

### Takeaway
Raw token confidence in code LLMs is poorly calibrated: Spiess et al. (ICSE 2025) report ECE of 0.09 to 0.73. Token-entropy signals give moderate AUROC, about 0.70 or above on Python LiveCodeBench but often 0.55 to 0.70 on Java. Code-only top-K token entropy reaches about 0.716 AUROC in one 2026 study, close to multi-sample baselines at a third of the cost.

### Cited Findings
- Spiess, Gros, et al., "Calibration and Correctness of Language Models for Code", ICSE 2025 (DOI 10.1109/ICSE55347.2025.00040; arXiv 2402.02047).
  - "Generally high ECE across all settings, ranging from 0.09 to 0.73"; intrinsic confidences are poor predictors of correctness.
  - Platt rescaling with known labels can cut ECE sharply. One table shows 0.46 falling to 0.04 at pass@1 = 33% **[verify task/model]**.
  - The authors argue that low ECE is more useful than high AUC alone for graded quality-control effort.
  - Source: [software-lab ICSE PDF](https://software-lab.org/publications/icse2025_calibration.pdf); [arXiv](https://arxiv.org/pdf/2402.02047v3); [ICSE page](https://conf.researchr.org/details/icse-2025/icse-2025-research-track/8/Calibration-and-Correctness-of-Language-Models-for-Code)
- "Functional Entropy: Predicting Functional Correctness in LLM-Generated Code with Uncertainty Quantification" (arXiv 2605.28500, 2026).
  - Covers 3 languages, 5 LLMs and over 1,700 problems, with AUROC as the main metric.
  - NLI-based sampling methods break down for code.
  - Most methods exceed 0.70 AUROC on LiveCodeBench (Python) but mostly fall between 0.55 and 0.70 on MultiPL-E (Java). Minimum token probability is the best Java method on one model, at 0.705.
  - It proposes "functional entropy", an LLM-judged analogue of semantic entropy.
  - Source: [arXiv](https://arxiv.org/html/2605.28500v1)
- Shi et al., "Code Is More Than Text: Uncertainty Estimation for Code Generation" (arXiv 2606.09577, 2026).
  - Proposes three axes: lexical (Top-K token entropy, mean of the 5 largest), algorithmic (pseudo-code consistency) and functional (behavioural consistency on generated tests).
  - The ensemble raises average AUROC across 5 code LLMs from 0.696 (best NL-derived baseline) to 0.776.
  - Code-only token entropy scores 0.716 AUROC; comment-only entropy scores 0.375 (worse than random).
  - On Qwen3-14B, single-pass Top-K entropy matches the best multi-pass baseline at more than 3x lower cost.
  - MBPP / Qwen3-Coder-30B ablation: full method 0.7478 AUROC; without generated tests 0.6172.
  - Source: [arXiv](https://arxiv.org/html/2606.09577)
- Sharma & David found that LLM token probabilities add little to symbolic-cluster entropy, since a uniform distribution works about as well — [arXiv 2502.11620](https://arxiv.org/abs/2502.11620)

### Inferences
- Token log-probability and entropy are cheap, mandatory baselines. Expect AUROC around 0.6 to 0.72, and only score code tokens, because comment tokens are anti-correlated with correctness.
- The functional axis (execution agreement) is what lifts the ensemble AUROC (0.75 with tests vs 0.62 without, in the ablation). This supports making execution behaviour the core signal.

### Gaps
- Spiess et al. AUROC per model/task and the Huang et al. "Look Before You Leap" code-uncertainty numbers were not retrieved.

## Q4. Learned / probing detectors and static-analysis signals

### Takeaway
White-box probes on hidden states (OPENIA and successors) predict code correctness better than CodeBERT/CodeT5+ classifiers. They need open weights and labelled training data, and a strong external LLM judge can beat them.

### Cited Findings
- Bui et al., OPENIA, "Correctness Assessment of Code Generated by LLMs Using Internal Representations" (arXiv 2501.12934; Journal of Systems and Software 2025).
  - Probes intermediate states (last generated token, final layer) of DeepSeek-Coder, Code Llama and Magicoder.
  - Reports "up to a 2X improvement in accuracy compared to zero-shot baselines" and up to 20% higher accuracy than CodeBERT/CodeT5+ classifiers.
  - Repo-level gains are given as 46% (arXiv v1) or 3X (journal) **[verify]**.
  - Example, DeepSeek-Coder-1.3B standalone F1: OPENIA 0.66, in-house LLM judge 0.50, CodeBERT 0.45, CodeT5+ 0.34. An external LLM judge reached 0.93 **[verify]**.
  - Source: [arXiv](https://arxiv.org/html/2501.12934v1); [ScienceDirect](https://www.sciencedirect.com/science/article/abs/pii/S0164121225002390); code at [OPENIA](https://github.com/iSE-UET-VNU/OPENIA)
- Follow-up probing papers:
  - AutoProbe (arXiv 2510.02934) uses attention-weighted hidden states across layers and tokens, and covers compilability, functionality and security — [arXiv](https://arxiv.org/pdf/2510.02934)
  - "On LLMs' Internal Representation of Code Correctness" (arXiv 2512.07404) reports gains over log-likelihood ranking and verbalised confidence — [arXiv](https://arxiv.org/abs/2512.07404)
  - "Code Correctness Is Linearly Decodable from LLM Hidden States Before Generation" (arXiv 2606.14530) — [arXiv](https://arxiv.org/pdf/2606.14530)
  - "On the Robustness of LLMs' Internal Representation of Code Correctness" (arXiv 2608.08266) — [arXiv](https://arxiv.org/pdf/2608.08266)

### Inferences
- Probes are a strong but white-box, supervised comparator. The planned paper's black-box, unsupervised behavioural-consistency score should be positioned against them, or a probe included as an upper-bound baseline on an open model.

### Gaps
- No AUROC numbers for OPENIA were retrieved.
- Static-analysis signals (linters, type checkers, compile errors) as correctness predictors were not researched within the tool budget. No source-backed numbers are available.

## Q5. Generating test inputs without expected outputs

### Takeaway
The standard recipe:
1. Get seed inputs from an LLM (or from types and signatures).
2. Expand them with type-aware mutation (EvalPlus).
3. Compare program outputs differentially, or by agreement / dual-execution agreement (CodeT), with no expected outputs.

### Cited Findings
- EvalPlus: Liu et al., "Is Your Code Generated by ChatGPT Really Correct?", NeurIPS 2023 (arXiv 2305.01210).
  - ChatGPT produces seed inputs, then type-aware mutation expands them: int/float ±1; list items removed, repeated or replaced; substrings removed or repeated.
  - Outputs are checked differentially against a reference.
  - HumanEval+ has 80x more tests than HumanEval, and pass@k drops by up to 19.3 to 28.9% across 26 LLMs.
  - A set-cover step reduces the suite.
  - Source: [NeurIPS](https://nips.cc/virtual/2023/poster/72990); [ar5iv](https://ar5iv.labs.arxiv.org/html/2305.01210); [poster](https://jiaweiliu.web.illinois.edu/assets/pdf/EvalPlus_Poster.pdf)
- CodeT: Chen et al., "Code Generation with Generated Tests" (arXiv 2207.10397; ICLR 2023).
  - The LLM generates both code and tests. "Dual execution agreement" rewards agreement with the generated tests and with other samples.
  - HumanEval pass@1 rises to 65.8%, +18.8 absolute over code-davinci-002.
  - Source: [arXiv PDF](https://arxiv.org/pdf/2207.10397)
- Functional clustering (2506.11021) and Code-MUE (2607.12273) use self-generated test inputs or probe inputs with no expected outputs, and compare only program outputs — [arXiv](https://arxiv.org/abs/2506.11021); [arXiv](https://arxiv.org/abs/2607.12273)

### Inferences
- For the planned paper, EvalPlus-style inputs are a defensible input source: LLM seeds plus type-aware mutation, with ground-truth tests held out for labels only. Run programs from the original and perturbed prompts on them and compute agreement.
- Report how sensitive AUROC is to the number and source of inputs.

### Gaps
- Pure type-based fuzzing (no LLM seeds) for code-LLM agreement was not found with numbers.

## Q6. Strongest known test-free predictor and novelty gap

### Takeaway
The strongest reported test-free signals are execution-behaviour-agreement methods:
- functional clustering: error rate 65% to 2% on LiveCodeBench at abstention
- incoherence: about 2/3 of errors caught with no reported false positives
- Code-MUE: Spearman up to -0.98
- 3-axis ensemble: AUROC 0.776

The best AUROCs reported for single-model token signals are about 0.70 to 0.72.

**Novelty gap:** prompt paraphrasing plus execution cross-validation already exists (Wang & Zhu 2024 / ICSE 2026 NIER), and so does problem transformation plus hyperproperty agreement (Semantic Triangulation, OOPSLA 2026). No paper was found that:
1. uses graded character/word/sentence-level perturbations (Saha-style) as the source of variation;
2. scores continuous behavioural agreement on shared inputs as a per-sample hallucination score; and
3. benchmarks it by AUROC against semantic entropy (symbolic or functional), token entropy and probes.

### Cited Findings
- Closest prior: metamorphic / paraphrase prompt testing. Paraphrases plus cross-validation of programs: 75% detection at 8.6% FPR (GPT-4, HumanEval); 70.6% at 6.1% FPR (GPT-4o, 5 paraphrases) — [arXiv 2406.06864](https://arxiv.org/abs/2406.06864); [ACM](https://dl.acm.org/doi/10.1145/3786582.3786809)
- Second-closest: Semantic Triangulation, built on non-equivalent problem transformations — [arXiv 2511.12288](https://arxiv.org/html/2511.12288)
- Same-prompt execution-agreement detectors: [incoherence 2507.00057](https://arxiv.org/abs/2507.00057), [functional clustering 2506.11021](https://arxiv.org/abs/2506.11021), [Code-MUE 2607.12273](https://arxiv.org/abs/2607.12273), [symbolic clustering 2502.11620](https://arxiv.org/abs/2502.11620)
- Best ensemble AUROC found: 0.776 average across 5 LLMs — [arXiv 2606.09577](https://arxiv.org/html/2606.09577)
- Perturbation-based UQ for text (not code): [SPUQ, EACL 2024](https://aclanthology.org/2024.eacl-long.143.pdf); [Saha 2025](https://www.researchgate.net/publication/392213537_You_believe_your_LLM_is_not_delusional_Think_again_a_study_of_LLM_hallucination_on_foundation_models_under_perturbation)

### Inferences
- The paper's contribution has to be framed as an extension, not a first. Plain "paraphrase plus execution consistency" is taken by Wang & Zhu. Defensible novelty:
  - (a) a perturbation taxonomy and granularity from Saha/ReCode/NLPerturbator, plus an analysis of which perturbation levels carry the most signal;
  - (b) a continuous agreement score evaluated by AUROC and AUPRC across several models and benchmarks (Wang & Zhu use small binary-detection evaluations);
  - (c) a head-to-head against same-prompt resampling (incoherence, functional clustering, symbolic semantic entropy), to test whether perturbation adds information beyond temperature sampling;
  - (d) combination with token-entropy features.
- A key ablation: perturbed-prompt agreement vs same-prompt resampling agreement at equal sample budgets.

### Gaps
- Metrics differ across papers (detection rate/FPR, error-at-abstention, Spearman, AUROC), so no single leaderboard of test-free predictors exists. Whether any paper published after mid-2026 already reports AUROC for paraphrase-based execution consistency could not be fully ruled out within the search budget.
