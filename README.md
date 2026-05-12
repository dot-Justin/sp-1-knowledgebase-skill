# SP-1 Knowledgebase Skill

A Claude Code / Claude Agent skill containing a curated, cited reference library for the **Teenage Engineering SP-1 stem player** — the unreleased Kanye West / TE prototype built on the nRF52840.

The community reverse-engineered this device over 3+ years (2024-04 to 2026-05+). This skill synthesizes their work into accurate, source-linked reference files so an agent can answer technical questions about the device without hallucinating.

## Entry point

Start at [`SKILL.md`](./SKILL.md). It contains the navigation, the quick-lookup answers, and the protocol the agent must follow when working on SP-1 topics.

## Accuracy anchors

Before asserting any technical claim, the agent consults:

- [`hallucination-watchlist.md`](./hallucination-watchlist.md) — past AI errors publicly called out by the community
- [`corrections.md`](./corrections.md) — documented reversals (what was once believed wrong)
- [`known-unknowns.md`](./known-unknowns.md) — what is explicitly unresolved as of synthesis date
- [`working-confirmed.md`](./working-confirmed.md) — safe-claims list with citations
- [`sources.md`](./sources.md) — where to find primary sources

The synthesis log ([`synthesis-log.md`](./synthesis-log.md)) records what was incorporated when. Each commit to this repo reflects a refinement to that synthesis.

## Install locally

Clone into your Claude skills directory:

```sh
git clone https://github.com/dot-Justin/sp-1-knowledgebase-skill ~/.claude/skills/sp-1
```

## Maintained by

[@dot-Justin](https://github.com/dot-Justin) — see also the [Lines thread archive](https://github.com/dot-Justin/TE-SP-1-lines-thread-archive) (web frontend at [sp-1.dotjust.in](https://sp-1.dotjust.in)).

## Citing the community

This skill is a derivative work of community research on Lines (`llllllll.co`) and the SP-1 Discord. Key contributors named throughout the references include TimK / tkt1000 / timknapen, ericlewis, murray, Galapagoose, JoseJX, B_E_N, emvee1968, virtualflannel_46386, moecal1947, fishdog_, zee_33, theunflappable, Duloz, and many others. Citations point back to the original posts and code.

The bundled TE manuals in [`assets/`](./assets/) are the only redistributed material. Community-private artifacts (stock TE firmware, Kanye stems, `audiothingies.zip` / `storagethingies.zip`) are described but not included.
